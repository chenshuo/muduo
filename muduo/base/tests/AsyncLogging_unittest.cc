// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#include "muduo/base/AsyncLogging.h"

#include <atomic>
#include <fstream>
#include <iterator>
#include <thread>

#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace
{

void check(bool condition, const char* expression, int line)
{
  if (!condition)
  {
    fprintf(stderr, "AsyncLogging_unittest.cc:%d: %s\n", line, expression);
    abort();
  }
}

#define CHECK(expression) check((expression), #expression, __LINE__)

enum Gate { kOpen, kWrite };

struct GateState
{
  explicit GateState(Gate gateArg)
    : gate(gateArg), entered(1), release(1), stopNotified(1),
      blocked(false), observeStop(false), stopped(false), waitsAfterStop(0)
  {
  }

  void blockOnce()
  {
    if (!blocked.exchange(true))
    {
      entered.countDown();
      release.wait();
    }
  }

  const Gate gate;
  muduo::CountDownLatch entered;
  muduo::CountDownLatch release;
  muduo::CountDownLatch stopNotified;
  std::atomic<bool> blocked;
  std::atomic<bool> observeStop;
  std::atomic<bool> stopped;
  std::atomic<int> waitsAfterStop;
};

std::atomic<GateState*> activeGate(nullptr);

void stopWhileBlocked(muduo::AsyncLogging& log, GateState& state)
{
  // Observe stop()'s actual condition notification before releasing the writer.
  // This fixes the interleaving without scheduler sleeps or production hooks.
  state.observeStop = true;
  std::thread stopper([&log] { log.stop(); });
  state.stopNotified.wait();
  state.release.countDown();
  stopper.join();
  activeGate = nullptr;
  CHECK(state.waitsAfterStop == 0);
}

void checkLog(const muduo::string& basename, const muduo::string& expected)
{
  glob_t files = {};
  CHECK(::glob((basename + ".*.log").c_str(), 0, NULL, &files) == 0);
  CHECK(files.gl_pathc == 1);
  std::ifstream input(files.gl_pathv[0]);
  const muduo::string contents((std::istreambuf_iterator<char>(input)),
                              std::istreambuf_iterator<char>());
  CHECK(contents == expected);
  input.close();
  CHECK(::unlink(files.gl_pathv[0]) == 0);
  ::globfree(&files);
}

void testStartup(bool withMessage)
{
  GateState state(kOpen);
  muduo::AsyncLogging log("startup", 64*1024*1024, 1);
  activeGate = &state;
  log.start();
  state.entered.wait();
  const muduo::string message = withMessage ? "before immediate stop\n" : "";
  log.append(message.data(), static_cast<int>(message.size()));
  stopWhileBlocked(log, state);
  checkLog("startup", message);
}

void testInflightWrite()
{
  GateState state(kWrite);
  muduo::AsyncLogging log("inflight", 64*1024*1024, 1);
  activeGate = &state;
  log.start();

  // Fill and roll a buffer, then pause the writer outside mutex_.
  const muduo::string first(muduo::detail::kLargeBuffer - 1, 'a');
  const muduo::string second = "second buffer\n";
  const muduo::string final = "appended while the previous batch is writing\n";
  log.append(first.data(), static_cast<int>(first.size()));
  log.append(second.data(), static_cast<int>(second.size()));
  state.entered.wait();
  log.append(final.data(), static_cast<int>(final.size()));
  stopWhileBlocked(log, state);
  checkLog("inflight", first + second + final);
}

}  // namespace

// GNU ld wrappers affect this test executable only. All file I/O and pthread
// operations still use their real implementations after synchronization.
extern "C"
{
FILE* __real_fopen(const char*, const char*);
size_t __real_fwrite_unlocked(const void*, size_t, size_t, FILE*);
int __real_pthread_cond_signal(pthread_cond_t*);
int __real_pthread_cond_timedwait(pthread_cond_t*, pthread_mutex_t*, const timespec*);

FILE* __wrap_fopen(const char* filename, const char* mode)
{
  GateState* state = activeGate.load();
  if (state && state->gate == kOpen)
  {
    state->blockOnce();
  }
  return __real_fopen(filename, mode);
}

size_t __wrap_fwrite_unlocked(const void* data, size_t size, size_t count, FILE* file)
{
  GateState* state = activeGate.load();
  if (state && state->gate == kWrite)
  {
    state->blockOnce();
  }
  return __real_fwrite_unlocked(data, size, count, file);
}

int __wrap_pthread_cond_signal(pthread_cond_t* cond)
{
  const int result = __real_pthread_cond_signal(cond);
  GateState* state = activeGate.load();
  if (state && state->observeStop.exchange(false))
  {
    state->stopped = true;
    state->stopNotified.countDown();
  }
  return result;
}

int __wrap_pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex,
                                const timespec* abstime)
{
  GateState* state = activeGate.load();
  if (state && state->stopped)
  {
    ++state->waitsAfterStop;
  }
  return __real_pthread_cond_timedwait(cond, mutex, abstime);
}
}  // extern "C"

int main(int argc, char* argv[])
{
  CHECK(argc == 2);
  char directory[] = "/tmp/muduo-async-shutdown-XXXXXX";
  CHECK(::mkdtemp(directory) != NULL);
  CHECK(::chdir(directory) == 0);

  const muduo::string scenario = argv[1];
  if (scenario == "empty")
    testStartup(false);
  else if (scenario == "startup")
    testStartup(true);
  else
  {
    CHECK(scenario == "inflight");
    testInflightWrite();
  }

  CHECK(::chdir("/") == 0);
  CHECK(::rmdir(directory) == 0);
}
