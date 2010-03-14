#ifndef MUDUO_NET_POLLPOLLER_H
#define MUDUO_NET_POLLPOLLER_H

#include <muduo/net/Poller.h>

#include <map>
#include <vector>

#include <poll.h>

namespace muduo
{
namespace net
{

///
/// IO Multiplexing with poll(2).
///
class PollPoller : public Poller
{
 public:

  virtual ~PollPoller();

  virtual void poll(int timeoutMs, ChannelList* activeChannels);
  virtual void updateChannel(Channel* channel);

 private:
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  typedef std::vector<struct pollfd> PollFdList;
  typedef std::map<int, Channel*> ChannelMap;
  PollFdList pollfds_;
  ChannelMap channels_;
};

}
}
#endif  // MUDUO_NET_POLLPOLLER_H
