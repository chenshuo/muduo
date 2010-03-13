#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

///
/// Base class for IO Multiplexing
///
class Poller : boost::noncopyable
{
 public:

  virtual ~Poller();

  virtual void poll(int timeoutMs) = 0;

  static Poller* newDefaultPoller();
};

}
}
#endif
