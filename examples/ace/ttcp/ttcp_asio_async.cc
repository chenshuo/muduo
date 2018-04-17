#include <examples/ace/ttcp/common.h>

#include <muduo/base/Logging.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
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

class TtcpServerConnection : boost::noncopyable,
  public boost::enable_shared_from_this<TtcpServerConnection>
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
    boost::asio::async_read(
        socket_, boost::asio::buffer(&sessionMessage_, sizeof(sessionMessage_)),
        boost::bind(&TtcpServerConnection::handleReadSession, shared_from_this(),
                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
  }

 private:
  void handleReadSession(const boost::system::error_code& error, size_t len)
  {
    if (!error && len == sizeof sessionMessage_)
    {
      sessionMessage_.number = ntohl(sessionMessage_.number);
      sessionMessage_.length = ntohl(sessionMessage_.length);
      printf("receive number = %d\nreceive length = %d\n",
             sessionMessage_.number, sessionMessage_.length);
      const int total_len = static_cast<int>(sizeof(int32_t) + sessionMessage_.length);
      payload_ = static_cast<PayloadMessage*>(::malloc(total_len));
      payload_->length = 0;
      boost::asio::async_read(
          socket_, boost::asio::buffer(&payload_->length, sizeof payload_->length),
          boost::bind(&TtcpServerConnection::handleReadLength, shared_from_this(),
                      boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      LOG_ERROR << "read session message: " << error.message();
    }
  }

  void handleReadLength(const boost::system::error_code& error, size_t len)
  {
    if (!error && len == sizeof payload_->length)
    {
      payload_->length = ntohl(payload_->length);
      assert(payload_->length == sessionMessage_.length);
      boost::asio::async_read(
          socket_, boost::asio::buffer(&payload_->data, payload_->length),
          boost::bind(&TtcpServerConnection::handleReadPayload, shared_from_this(),
                      boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      LOG_ERROR << "read length: " << error.message();
    }
  }

  void handleReadPayload(const boost::system::error_code& error, size_t len)
  {
    if (!error && len == static_cast<size_t>(payload_->length))
    {
      ack_ = htonl(payload_->length);
      boost::asio::async_write(
          socket_, boost::asio::buffer(&ack_, sizeof ack_),
          boost::bind(&TtcpServerConnection::handleWriteAck, shared_from_this(),
                      boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      LOG_ERROR << "read payload data: " << error.message();
    }
  }

  void handleWriteAck(const boost::system::error_code& error, size_t len)
  {
    if (!error && len == sizeof ack_)
    {
      if (++count_ < sessionMessage_.number)
      {
        payload_->length = 0;
        boost::asio::async_read(
            socket_, boost::asio::buffer(&payload_->length, sizeof payload_->length),
            boost::bind(&TtcpServerConnection::handleReadLength, shared_from_this(),
                        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
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
  }

  tcp::socket socket_;
  int count_;
  struct SessionMessage sessionMessage_;
  struct PayloadMessage* payload_;
  int32_t ack_;
};
typedef boost::shared_ptr<TtcpServerConnection> TtcpServerConnectionPtr;

void startAccept(tcp::acceptor& acceptor);

void handleAccept(tcp::acceptor& acceptor, TtcpServerConnectionPtr new_connection,
                  const boost::system::error_code& error)
{
  if (!error)
  {
    new_connection->start();
  }

  startAccept(acceptor);
}

void startAccept(tcp::acceptor& acceptor)
{
  TtcpServerConnectionPtr new_connection(new TtcpServerConnection(acceptor.get_io_service()));
  acceptor.async_accept(new_connection->socket(),
                         boost::bind(handleAccept, boost::ref(acceptor), new_connection,
                                     boost::asio::placeholders::error));
}

void receive(const Options& opt)
{
  try
  {
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), opt.port));
    startAccept(acceptor);
    io_service.run();
  }
  catch (std::exception& e)
  {
    LOG_ERROR << e.what();
  }
}

