// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo
{
class Timestamp;
namespace net
{

// All client visible callbacks go here.

class ChannelBuffer;
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::function<void()> TimerCallback;
typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;

// the data has been read to (buf, len)
typedef boost::function<void (const TcpConnectionPtr&,
                              ChannelBuffer*,
                              Timestamp receiveTime)> MessageCallback;

}
}

#endif  // MUDUO_NET_CALLBACKS_H
