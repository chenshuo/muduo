// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H

#include <muduo/base/Mutex.h>
#include <muduo/base/Types.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/ChannelBuffer.h>
#include <muduo/net/InetAddress.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo
{
namespace net
{

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class TcpConnection : public boost::enable_shared_from_this<TcpConnection>,
                      boost::noncopyable
{
 public:
  /// Constructs a TcpConnection with a connected sockfd
  ///
  TcpConnection(EventLoop* loop,
                const string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const string& name() const { return name_; }
  const InetAddress& localAddress() { return localAddr_; }
  const InetAddress& peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }

  void send(const string& message);
  // void send(const ChannelBuffer& message);
  void send(ChannelBuffer* message);  // this one will swap data
  void shutdown();

  void setConnectionCallback(ConnectionCallback cb)
  { connectionCallback_ = cb; }

  void setMessageCallback(MessageCallback cb)
  { messageCallback_ = cb; }

  /// Internal use only.
  void setCloseCallback(ConnectionCallback cb)
  { closeCallback_ = cb; }

  // called when TcpServer accepts a new connection
  void connectEstablished();
  void connectDestroyed();

 private:
  enum States { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const string& message);
  void setState(States s) { state_ = s; }

  EventLoop* loop_;
  string name_;
  States state_;
  // we don't expose those classes to client.
  boost::scoped_ptr<Socket> socket_;
  boost::scoped_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  ConnectionCallback closeCallback_;
  ChannelBuffer inputBuffer_;
  // MutexLock mutex_;
  ChannelBuffer outputBuffer_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}

#endif  // MUDUO_NET_TCPCONNECTION_H
