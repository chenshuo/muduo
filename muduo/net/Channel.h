#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel : boost::noncopyable
{
 public:
  typedef boost::function<void()> EventCallback;
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;
  static const int kErrorvent;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handle_event();
  void setReadCallback(const EventCallback& cb)
  { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb)
  { writeCallback_ = cb; }
  void setCloseCallback(const EventCallback& cb)
  { closeCallback_ = cb; }
  void setErrorCallback(const EventCallback& cb)
  { errorCallback_ = cb; }

  int fd() { return fd_; }
  int events() { return events_; }
  void set_events(int evt) { events_ = evt; }
  void set_revents(int revt) { revents_ = revt; }

  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop* getLoop() { return loop_; }

  // void set_loop(EventLoop* loop) { loop_ = loop; }

 private:
  EventLoop* loop_;
  const int  fd_;
  int        events_;
  int        revents_;
  int        index_; // used by PollPoller.
  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}
}
#endif  // MUDUO_NET_CHANNEL_H
