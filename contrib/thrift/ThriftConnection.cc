#include "ThriftConnection.h"

#include <boost/bind.hpp>

#include <muduo/base/Logging.h>

#include <thrift/transport/TTransportException.h>

#include "ThriftServer.h"

using namespace muduo;
using namespace muduo::net;

ThriftConnection::ThriftConnection(ThriftServer* server,
                                  const TcpConnectionPtr& conn)
  : server_(server),
    conn_(conn),
    state_(kExpectFrameSize),
    frameSize_(0)
{
  conn_->setMessageCallback(boost::bind(&ThriftConnection::onMessage,
                                        this, _1, _2, _3));
  nullTransport_.reset(new TNullTransport());
  inputTransport_.reset(new TMemoryBuffer(NULL, 0));
  outputTransport_.reset(new TMemoryBuffer());

  factoryInputTransport_ = server_->getInputTransportFactory()->getTransport(inputTransport_);
  factoryOutputTransport_ = server_->getOutputTransportFactory()->getTransport(outputTransport_);

  inputProtocol_ = server_->getInputProtocolFactory()->getProtocol(factoryInputTransport_);
  outputProtocol_ = server_->getOutputProtocolFactory()->getProtocol(factoryOutputTransport_);

  processor_ = server_->getProcessor(inputProtocol_, outputProtocol_, nullTransport_);
}

void ThriftConnection::onMessage(const TcpConnectionPtr& conn,
                                 Buffer* buffer,
                                 Timestamp receiveTime)
{
  bool more = true;
  while (more)
  {
    if (state_ == kExpectFrameSize)
    {
      if (buffer->readableBytes() >= 4)
      {
        frameSize_ = static_cast<uint32_t>(buffer->readInt32());
        state_ = kExpectFrame;
      }
      else
      {
        more = false;
      }
    }
    else if (state_ == kExpectFrame)
    {
      if (buffer->readableBytes() >= frameSize_)
      {
        uint8_t* buf = reinterpret_cast<uint8_t*>((const_cast<char*>(buffer->peek())));

        inputTransport_->resetBuffer(buf, frameSize_, TMemoryBuffer::COPY);
        outputTransport_->resetBuffer();
        outputTransport_->getWritePtr(4);
        outputTransport_->wroteBytes(4);

        if (server_->isWorkerThreadPoolProcessing())
        {
          server_->workerThreadPool().run(boost::bind(&ThriftConnection::process, this));
        }
        else
        {
          process();
        }

        buffer->retrieve(frameSize_);
        state_ = kExpectFrameSize;
      }
      else
      {
        more = false;
      }
    }
  }
}

void ThriftConnection::process()
{
  try
  {
    processor_->process(inputProtocol_, outputProtocol_, NULL);

    uint8_t* buf;
    uint32_t size;
    outputTransport_->getBuffer(&buf, &size);

    assert(size >= 4);
    uint32_t frameSize = static_cast<uint32_t>(htonl(size - 4));
    memcpy(buf, &frameSize, 4);

    conn_->send(buf, size);
  } catch (const TTransportException& ex)
  {
    LOG_ERROR << "ThriftServer TTransportException: " << ex.what();
    close();
  } catch (const std::exception& ex)
  {
    LOG_ERROR << "ThriftServer std::exception: " << ex.what();
    close();
  } catch (...)
  {
    LOG_ERROR << "ThriftServer unknown exception";
    close();
  }
}

void ThriftConnection::close()
{
  nullTransport_->close();
  factoryInputTransport_->close();
  factoryOutputTransport_->close();
}
