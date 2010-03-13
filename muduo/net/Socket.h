#ifndef MUDUO_NET_SOCKET_H
#define MUDUO_NET_SOCKET_H

namespace muduo
{
///
/// TCP networking.
///
namespace net
{

///
/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
///
class Socket
{
 public:
  explicit Socket(int sockfd)
    : sockfd_(sockfd)
  { }

  ~Socket();

  int fd() { return sockfd_; }

  ///
  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  ///
  void setTcpNoDelay(bool on);

 private:
  int sockfd_;
};

}
}
#endif
