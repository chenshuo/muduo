#include <muduo/base/FileUtil.h>
#include <muduo/base/ProcessInfo.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>
#include <muduo/net/http/HttpServer.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/bind.hpp>

#include <sstream>
#include <stdarg.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

// TODO:
// - what if process exits?
//

class Procmon : boost::noncopyable
{
 public:
  Procmon(EventLoop* loop, pid_t pid, uint16_t port, const char* procname)
    : kClockTicksPerSecond_(muduo::ProcessInfo::clockTicksPerSecond()),
      kbPerPage_(muduo::ProcessInfo::pageSize() / 1024),
      kBootTime_(getBootTime()),
      pid_(pid),
      server_(loop, InetAddress(port), getName()),
      procname_(ProcessInfo::procname(readProcFile("stat")).as_string()),
      hostname_(ProcessInfo::hostname()),
      cmdline_(getCmdLine())
  {
    server_.setHttpCallback(boost::bind(&Procmon::onRequest, this, _1, _2));
  }

  void start()
  {
    server_.start();
  }

 private:

  string getName() const
  {
    char name[256];
    snprintf(name, sizeof name, "procmon-%d", pid_);
    return name;
  }

  void onRequest(const HttpRequest& req, HttpResponse* resp)
  {
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/plain");
    resp->addHeader("Server", "Muduo-Procmon");
    if (req.path() == "/")
    {
      resp->setContentType("text/html");
      fillOverview();
      resp->setBody(response_.retrieveAllAsString());
    }
    else if (req.path() == "/cmdline")
    {
      resp->setBody(cmdline_);
    }
    // FIXME: replace with a map
    else if (req.path() == "/environ")
    {
      resp->setBody(getEnviron());
    }
    else if (req.path() == "/io")
    {
      resp->setBody(readProcFile("io"));
    }
    else if (req.path() == "/limits")
    {
      resp->setBody(readProcFile("limits"));
    }
    else if (req.path() == "/maps")
    {
      resp->setBody(readProcFile("maps"));
    }
    // numa_maps
    else if (req.path() == "/smaps")
    {
      resp->setBody(readProcFile("smaps"));
    }
    else if (req.path() == "/status")
    {
      resp->setBody(readProcFile("status"));
    }
    else if (req.path() == "/threads")
    {
      fillThreads();
      resp->setBody(response_.retrieveAllAsString());
    }
    else
    {
      resp->setStatusCode(HttpResponse::k404NotFound);
      resp->setStatusMessage("Not Found");
      resp->setCloseConnection(true);
    }
  }

  void fillOverview()
  {
    response_.retrieveAll();
    Timestamp now = Timestamp::now();
    appendResponse("<html><head><title>%s on %s</title></head><body>\n",
                   procname_.c_str(), hostname_.c_str());

    //            0    1    2    3     4    5       6   7 8 9  11  13   15
    // 3770 (cat) R 3718 3770 3718 34818 3770 4202496 214 0 0 0 0 0 0 0 20
    // 16  18     19      20 21                   22      23      24              25
    //  0 1 0 298215 5750784 81 18446744073709551615 4194304 4242836 140736345340592
    //              26
    // 140736066274232 140575670169216 0 0 0 0 0 0 0 17 0 0 0 0 0 0
    string stat = readProcFile("stat");
    int pid = atoi(stat.c_str());
    assert(pid == pid_);
    StringPiece procname = ProcessInfo::procname(stat);
    appendResponse("<h1>%s on %s</h1>\n",
                   procname.as_string().c_str(), hostname_.c_str());
    response_.append("<p><a href=\"/cmdline\">Command line</a>\n");
    response_.append("<a href=\"/environ\">Environment variables</a>\n");
    response_.append("<a href=\"/threads\">Threads</a>\n");

    appendResponse("<p>Page generated at %s (UTC)", now.toFormattedString().c_str());

    response_.append("<p><table>");
    // istringstream is probably not the most efficient way to parse it,
    // see muduo-protorpc/examples/collect/ProcFs.cc for alternatives.
    std::istringstream iss(procname.end()+1);  // end is ')'

    char state = ' ';
    iss >> state;

    {
    int ppid = 0, pgrp = 0, session = 0, tty_nr = 0, tpgid = 0, flags = 0;
    iss >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags;
    }

    long minflt = 0, majflt = 0;
    {
    long cminflt = 0, cmajflt = 0;
    iss >> minflt >> cminflt >> majflt >> cmajflt;
    }

    long utime = 0, stime = 0;
    {
    long cutime = 0, cstime = 0;
    iss >> utime >> stime >> cutime >> cstime;
    }

    long priority = 0, nice = 0, num_threads = 0, itrealvalue = 0, starttime = 0;
    iss >> priority >> nice >> num_threads >> itrealvalue >> starttime;

    long vsize = 0, rss = 0, rsslim = 0;
    iss >> vsize >> rss >> rsslim;

    appendTableRow("PID", pid);
    Timestamp started(getStartTime(starttime));  // FIXME: cache it;
    appendTableRow("Started at", started.toFormattedString() + " (UTC)");
    appendTableRowFloat("Uptime (s)", timeDifference(now, started));  // FIXME: format as days+H:M:S
    appendTableRow("Executable", readLink("exe"));
    appendTableRow("Current dir", readLink("cwd"));

    appendTableRow("State", getState(state));
    appendTableRowFloat("User time (s)", getSeconds(utime));
    appendTableRowFloat("System time (s)", getSeconds(stime));

    appendTableRow("VmSize (KiB)", vsize / 1024);
    appendTableRow("VmRSS (KiB)", rss * kbPerPage_);
    appendTableRow("Threads", num_threads);
    appendTableRow("Priority", priority);
    appendTableRow("Nice", nice);

    appendTableRow("Minor page faults", minflt);
    appendTableRow("Major page faults", majflt);
    // user
    response_.append("</table>");
    response_.append("</body></html>");
  }

  void fillThreads()
  {
    response_.retrieveAll();
    // FIXME
  }

  string readProcFile(const char* basename)
  {
    char filename[256];
    snprintf(filename, sizeof filename, "/proc/%d/%s", pid_, basename);
    string content;
    FileUtil::readFile(filename, 1024*1024, &content);
    return content;
  }

  string readLink(const char* basename)
  {
    char filename[256];
    snprintf(filename, sizeof filename, "/proc/%d/%s", pid_, basename);
    char link[1024];
    ssize_t len = ::readlink(filename, link, sizeof link);
    string result;
    if (len > 0)
    {
      result.assign(link, len);
    }
    return result;
  }

  int appendResponse(const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));

  void appendTableRow(const char* name, long value)
  {
    appendResponse("<tr><td>%s</td><td>%ld</td></tr>\n", name, value);
  }

  void appendTableRowFloat(const char* name, double value)
  {
    appendResponse("<tr><td>%s</td><td>%.2f</td></tr>\n", name, value);
  }

  void appendTableRow(const char* name, StringArg value)
  {
    appendResponse("<tr><td>%s</td><td>%s</td></tr>\n", name, value.c_str());
  }

  string getCmdLine()
  {
    return boost::replace_all_copy(readProcFile("cmdline"), string(1, '\0'), "\n\t");
  }

  string getEnviron()
  {
    return boost::replace_all_copy(readProcFile("environ"), string(1, '\0'), "\n");
  }

  Timestamp getStartTime(long starttime)
  {
  return Timestamp(Timestamp::kMicroSecondsPerSecond * kBootTime_
                   + Timestamp::kMicroSecondsPerSecond * starttime / kClockTicksPerSecond_);
  }

  double getSeconds(long ticks)
  {
    return static_cast<double>(ticks) / kClockTicksPerSecond_;
  }

  static const char* getState(char state)
  {
    // One  character  from  the  string  "RSDZTW"  where R is running, S is sleeping in an
    // interruptible wait, D is waiting in uninterruptible disk sleep, Z is  zombie,  T  is
    // traced or stopped (on a signal), and W is paging.
    switch (state)
    {
      case 'R':
        return "Running";
      case 'S':
        return "Sleeping";
      case 'D':
        return "Disk sleep";
      case 'Z':
        return "Zombie";
      default:
        return "Unknown";
    }
  }

  static long getLong(const string& status, const char* key)
  {
    long result = 0;
    size_t pos = status.find(key);
    if (pos != string::npos)
    {
      result = ::atol(status.c_str() + pos + strlen(key));
    }
    return result;
  }

  static long getBootTime()
  {
    string stat;
    FileUtil::readFile("/proc/stat", 65536, &stat);
    return getLong(stat, "btime ");
  }

  const int kClockTicksPerSecond_;
  const long kbPerPage_;
  const long kBootTime_;
  const pid_t pid_;
  HttpServer server_;
  const string procname_;
  const string hostname_;
  const string cmdline_;
  // scratch variables
  Buffer response_;
};

int Procmon::appendResponse(const char* fmt, ...)
{
  char buf[256];
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(buf, sizeof buf, fmt, args);
  va_end(args);
  response_.append(buf);
  return ret;
}

bool processExists(pid_t pid)
{
  char filename[256];
  snprintf(filename, sizeof filename, "/proc/%d/stat", pid);
  return ::access(filename, R_OK) == 0;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage: %s pid port [name]\n", argv[0]);
    return 0;
  }
  int pid = atoi(argv[1]);
  if (!processExists(pid))
  {
    printf("Process %d doesn't exist.\n", pid);
    return 1;
  }

  EventLoop loop;
  uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
  Procmon procmon(&loop, pid, port, argc > 3 ? argv[3] : "");
  procmon.start();
  loop.loop();
}
