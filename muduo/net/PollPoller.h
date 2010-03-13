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

 private:
  typedef std::vector<struct pollfd> PollFdList;
  PollFdList pollfds_;
  std::map<int, Channel*> channels_;
};

}
}
#endif
