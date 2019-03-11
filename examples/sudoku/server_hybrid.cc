#include "examples/sudoku/sudoku.h"

#include "muduo/base/Atomic.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/base/ThreadPool.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/inspect/Inspector.h"

#include <boost/circular_buffer.hpp>

//#include <stdio.h>
//#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

#include "examples/sudoku/stat.h"

class SudokuServer : noncopyable
{
 public:
  SudokuServer(EventLoop* loop,
               const InetAddress& listenAddr,
               int numEventLoops,
               int numThreads,
               bool nodelay)
    : server_(loop, listenAddr, "SudokuServer"),
      threadPool_(),
      numThreads_(numThreads),
      tcpNoDelay_(nodelay),
      startTime_(Timestamp::now()),
      stat_(threadPool_),
      inspectThread_(),
      inspector_(inspectThread_.startLoop(), InetAddress(9982), "sudoku-solver")
  {
    LOG_INFO << "Use " << numEventLoops << " IO threads.";
    LOG_INFO << "TCP no delay " << nodelay;

    server_.setConnectionCallback(
        std::bind(&SudokuServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&SudokuServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(numEventLoops);

    inspector_.add("sudoku", "stats", std::bind(&SudokuStat::report, &stat_),
                   "statistics of sudoku solver");
    inspector_.add("sudoku", "reset", std::bind(&SudokuStat::reset, &stat_),
                   "reset statistics of sudoku solver");
  }

  void start()
  {
    LOG_INFO << "Starting " << numThreads_ << " computing threads.";
    threadPool_.start(numThreads_);
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected() && tcpNoDelay_)
      conn->setTcpNoDelay(true);
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
  {
    LOG_DEBUG << conn->name();
    size_t len = buf->readableBytes();
    while (len >= kCells + 2)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        string request(buf->peek(), crlf);
        buf->retrieveUntil(crlf + 2);
        len = buf->readableBytes();
        stat_.recordRequest();
        if (!processRequest(conn, request, receiveTime))
        {
          conn->send("Bad Request!\r\n");
          conn->shutdown();
          stat_.recordBadRequest();
          break;
        }
      }
      else if (len > 100) // id + ":" + kCells + "\r\n"
      {
        conn->send("Id too long!\r\n");
        conn->shutdown();
        stat_.recordBadRequest();
        break;
      }
      else
      {
        break;
      }
    }
  }

  struct Request
  {
    string id;
    string puzzle;
    Timestamp receiveTime;
  };

  bool processRequest(const TcpConnectionPtr& conn, const string& request, Timestamp receiveTime)
  {
    Request req;
    req.receiveTime = receiveTime;

    string::const_iterator colon = find(request.begin(), request.end(), ':');
    if (colon != request.end())
    {
      req.id.assign(request.begin(), colon);
      req.puzzle.assign(colon+1, request.end());
    }
    else
    {
      // when using thread pool, an id must be provided in the request.
      if (numThreads_ > 1)
        return false;
      req.puzzle = request;
    }

    if (req.puzzle.size() == implicit_cast<size_t>(kCells))
    {
      threadPool_.run(std::bind(&SudokuServer::solve, this, conn, req));
      return true;
    }
    return false;
  }

  void solve(const TcpConnectionPtr& conn, const Request& req)
  {
    LOG_DEBUG << conn->name();
    string result = solveSudoku(req.puzzle);
    if (req.id.empty())
    {
      conn->send(result + "\r\n");
    }
    else
    {
      conn->send(req.id + ":" + result + "\r\n");
    }
    stat_.recordResponse(Timestamp::now(), req.receiveTime, result != kNoSolution);
  }

  TcpServer server_;
  ThreadPool threadPool_;
  const int numThreads_;
  const bool tcpNoDelay_;
  const Timestamp startTime_;

  SudokuStat stat_;
  EventLoopThread inspectThread_;
  Inspector inspector_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << argv[0] << " [number of IO threads] [number of worker threads] [-n]";
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  int numEventLoops = 0;
  int numThreads = 0;
  bool nodelay = false;
  if (argc > 1)
  {
    numEventLoops = atoi(argv[1]);
  }
  if (argc > 2)
  {
    numThreads = atoi(argv[2]);
  }
  if (argc > 3 && string(argv[3]) == "-n")
  {
    nodelay = true;
  }

  EventLoop loop;
  InetAddress listenAddr(9981);
  SudokuServer server(&loop, listenAddr, numEventLoops, numThreads, nodelay);

  server.start();

  loop.loop();
}

