#include "examples/ace/ttcp/common.h"

#include "muduo/base/Logging.h"
#include <boost/asio.hpp>
#include <stdio.h>

using boost::asio::ip::tcp;

void transmit(const Options& opt)
{
  try
  {
  }
  catch (std::exception& e)
  {
    LOG_ERROR << e.what();
  }
}

class TtcpServerConnection : public std::enable_shared_from_this<TtcpServerConnection>,
                             muduo::noncopyable
{
 public:
  TtcpServerConnection(boost::asio::io_service& io_service)
    : socket_(io_service), count_(0), payload_(NULL), ack_(0)
  {
    sessionMessage_.number = 0;
    sessionMessage_.length = 0;
  }

  ~TtcpServerConnection()
  {
    ::free(payload_);
  }

  tcp::socket& socket() { return socket_; }

  void start()
  {
    std::ostringstream oss;
    oss << socket_.remote_endpoint();
    LOG_INFO << "Got connection from " << oss.str();
    doReadSession();
  }

 private:
  void doReadSession()
  {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_, boost::asio::buffer(&sessionMessage_, sizeof(sessionMessage_)),
        [this, self](const boost::system::error_code& error, size_t len)
        {
          if (!error && len == sizeof sessionMessage_)
          {
            sessionMessage_.number = ntohl(sessionMessage_.number);
            sessionMessage_.length = ntohl(sessionMessage_.length);
            printf("receive number = %d\nreceive length = %d\n",
                   sessionMessage_.number, sessionMessage_.length);
            const int total_len = static_cast<int>(sizeof(int32_t) + sessionMessage_.length);
            payload_ = static_cast<PayloadMessage*>(::malloc(total_len));
            doReadLength();
          }
          else
          {
            LOG_ERROR << "read session message: " << error.message();
          }
        });
  }

  void doReadLength()
  {
    auto self(shared_from_this());
    payload_->length = 0;
    boost::asio::async_read(
        socket_, boost::asio::buffer(&payload_->length, sizeof payload_->length),
        [this, self](const boost::system::error_code& error, size_t len)
        {
          if (!error && len == sizeof payload_->length)
          {
            payload_->length = ntohl(payload_->length);
            doReadPayload();
          }
          else
          {
            LOG_ERROR << "read length: " << error.message();
          }
        });
  }

  void doReadPayload()
  {
    assert(payload_->length == sessionMessage_.length);
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_, boost::asio::buffer(&payload_->data, payload_->length),
        [this, self](const boost::system::error_code& error, size_t len)
        {
          if (!error && len == static_cast<size_t>(payload_->length))
          {
            doWriteAck();
          }
          else
          {
            LOG_ERROR << "read payload data: " << error.message();
          }
        });
  }

  void doWriteAck()
  {
    auto self(shared_from_this());
    ack_ = htonl(payload_->length);
    boost::asio::async_write(
        socket_, boost::asio::buffer(&ack_, sizeof ack_),
        [this, self](const boost::system::error_code& error, size_t len)
        {
          if (!error && len == sizeof ack_)
          {
            if (++count_ < sessionMessage_.number)
            {
              doReadLength();
            }
            else
            {
              LOG_INFO << "Done";
            }
          }
          else
          {
            LOG_ERROR << "write ack: " << error.message();
          }
        });
  }

  tcp::socket socket_;
  int count_;
  struct SessionMessage sessionMessage_;
  struct PayloadMessage* payload_;
  int32_t ack_;
};
typedef std::shared_ptr<TtcpServerConnection> TtcpServerConnectionPtr;

void doAccept(tcp::acceptor& acceptor)
{
  // no need to pre-create new_connection if we use asio 1.12 or boost 1.66+
  TtcpServerConnectionPtr new_connection(new TtcpServerConnection(acceptor.get_io_service()));
  acceptor.async_accept(
      new_connection->socket(),
      [&acceptor, new_connection](boost::system::error_code error)  // move new_connection in C++14
      {
        if (!error)
        {
          new_connection->start();
        }
        doAccept(acceptor);
      });
}

void receive(const Options& opt)
{
  try
  {
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), opt.port));
    doAccept(acceptor);
    io_service.run();
  }
  catch (std::exception& e)
  {
    LOG_ERROR << e.what();
  }
}

