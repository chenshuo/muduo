// Benchmark inspired by libevent/test/bench.c
// See also: http://libev.schmorp.de/bench.html

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <stdio.h>
#include <sys/resource.h>
#include <sys/socket.h>

using namespace muduo;
using namespace muduo::net;

std::vector<int> g_pipes;
int numPipes;
int numActive;
int numWrites;
EventLoop* g_loop;
boost::ptr_vector<Channel> g_channels;

int g_reads, g_writes, g_fired;

void readCallback(Timestamp, int fd, int idx)
{
  char ch;

  g_reads += static_cast<int>(::recv(fd, &ch, sizeof(ch), 0));
  if (g_writes > 0)
  {
    int widx = idx+1;
    if (widx >= numPipes)
    {
      widx -= numPipes;
    }
    ::send(g_pipes[2 * widx + 1], "m", 1, 0);
    g_writes--;
    g_fired++;
  }
  if (g_fired == g_reads)
  {
    g_loop->quit();
  }
}

std::pair<int, int> runOnce()
{
  Timestamp beforeInit(Timestamp::now());
  for (int i = 0; i < numPipes; ++i)
  {
    Channel& channel = g_channels[i];
    channel.setReadCallback(boost::bind(readCallback, _1, channel.fd(), i));
    channel.enableReading();
  }

  int space = numPipes / numActive;
  space *= 2;
  for (int i = 0; i < numActive; ++i)
  {
    ::send(g_pipes[i * space + 1], "m", 1, 0);
  }

  g_fired = numActive;
  g_reads = 0;
  g_writes = numWrites;
  Timestamp beforeLoop(Timestamp::now());
  g_loop->loop();

  Timestamp end(Timestamp::now());

  int iterTime = static_cast<int>(end.microSecondsSinceEpoch() - beforeInit.microSecondsSinceEpoch());
  int loopTime = static_cast<int>(end.microSecondsSinceEpoch() - beforeLoop.microSecondsSinceEpoch());
  return std::make_pair(iterTime, loopTime);
}

int main(int argc, char* argv[])
{
  numPipes = 100;
  numActive = 1;
  numWrites = 100;
  int c;
  while ((c = getopt(argc, argv, "n:a:w:")) != -1)
  {
    switch (c)
    {
      case 'n':
        numPipes = atoi(optarg);
        break;
      case 'a':
        numActive = atoi(optarg);
        break;
      case 'w':
        numWrites = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Illegal argument \"%c\"\n", c);
        return 1;
    }
  }

  struct rlimit rl;
  rl.rlim_cur = rl.rlim_max = numPipes * 2 + 50;
  if (::setrlimit(RLIMIT_NOFILE, &rl) == -1)
  {
    perror("setrlimit");
    //return 1;  // comment out this line if under valgrind
  }
  g_pipes.resize(2 * numPipes);
  for (int i = 0; i < numPipes; ++i)
  {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, &g_pipes[i*2]) == -1)
    {
      perror("pipe");
      return 1;
    }
  }

  EventLoop loop;
  g_loop = &loop;

  for (int i = 0; i < numPipes; ++i)
  {
    Channel* channel = new Channel(&loop, g_pipes[i*2]);
    g_channels.push_back(channel);
  }

  for (int i = 0; i < 25; ++i)
  {
    std::pair<int, int> t = runOnce();
    printf("%8d %8d\n", t.first, t.second);
  }

  for (boost::ptr_vector<Channel>::iterator it = g_channels.begin();
       it != g_channels.end(); ++it)
  {
    it->disableAll();
    it->remove();
  }
}
