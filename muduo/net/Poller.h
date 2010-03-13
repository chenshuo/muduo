#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <vector>
#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

class Channel;
///
/// Base class for IO Multiplexing
///
class Poller : boost::noncopyable
{
 public:
  typedef std::vector<Channel*> ChannelList;

  virtual ~Poller();

  virtual void poll(int timeoutMs, ChannelList* activeChannels) = 0;

  static Poller* newDefaultPoller();
};

}
}
#endif
