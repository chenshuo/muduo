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

void receive(const Options& opt)
{
  try
  {
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), opt.port));
    tcp::socket socket(io_service);
    acceptor.accept(socket);

    struct SessionMessage sessionMessage = { 0, 0 };
    boost::system::error_code error;
    size_t nr = boost::asio::read(socket, boost::asio::buffer(&sessionMessage, sizeof sessionMessage),
#if BOOST_VERSION < 104700L
                                  boost::asio::transfer_all(),
#endif
                                  error);
    if (nr != sizeof sessionMessage)
    {
      LOG_ERROR << "read session message: " << error.message();
      exit(1);
    }

    sessionMessage.number = ntohl(sessionMessage.number);
    sessionMessage.length = ntohl(sessionMessage.length);
    printf("receive number = %d\nreceive length = %d\n",
           sessionMessage.number, sessionMessage.length);
    const int total_len = static_cast<int>(sizeof(int32_t) + sessionMessage.length);
    PayloadMessage* payload = static_cast<PayloadMessage*>(::malloc(total_len));
    std::unique_ptr<PayloadMessage, void (*)(void*)> freeIt(payload, ::free);
    assert(payload);

    for (int i = 0; i < sessionMessage.number; ++i)
    {
      payload->length = 0;
      if (boost::asio::read(socket, boost::asio::buffer(&payload->length, sizeof(payload->length)),
#if BOOST_VERSION < 104700L
                            boost::asio::transfer_all(),
#endif
                            error) != sizeof(payload->length))
      {
        LOG_ERROR << "read length: " << error.message();
        exit(1);
      }
      payload->length = ntohl(payload->length);
      assert(payload->length == sessionMessage.length);
      if (boost::asio::read(socket, boost::asio::buffer(payload->data, payload->length),
#if BOOST_VERSION < 104700L
                            boost::asio::transfer_all(),
#endif
                            error) != static_cast<size_t>(payload->length))
      {
        LOG_ERROR << "read payload data: " << error.message();
        exit(1);
      }
      int32_t ack = htonl(payload->length);
      if (boost::asio::write(socket, boost::asio::buffer(&ack, sizeof(ack))) != sizeof(ack))
      {
        LOG_ERROR << "write ack: " << error.message();
        exit(1);
      }
    }
  }
  catch (std::exception& e)
  {
    LOG_ERROR << e.what();
  }
}
