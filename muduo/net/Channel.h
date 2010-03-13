#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include <boost/noncopyable.hpp>

#include <muduo/net/Socket.h>

namespace muduo
{
namespace net
{

class EventLoop;

class Channel : boost::noncopyable
{
 public:
  Channel(EventLoop* loop, Socket sock);
  ~Channel();

  void handle(int revents);

  /*
  int fd() { return fd_; }
  void set_fd(int _fd) { fd_ = _fd; }

  int events() { return events_; }
  void set_events(int events0) {  events_ = events0; }
  */

  EventLoop* getLoop() { return loop_; }

  // void set_loop(EventLoop* loop) { loop_ = loop; }

 private:
  EventLoop* loop_;
  Socket     sock_;
  int        events_;
};

}
}
#endif
