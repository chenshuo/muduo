#include <examples/curl/Curl.h>
#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>

#include <curl/curl.h>
#include <assert.h>

using namespace curl;
using namespace muduo;
using namespace muduo::net;

Request::Request(Curl* owner, StringPiece url)
  : owner_(owner),
    curl_(CHECK_NOTNULL(curl_easy_init()))
{
  setopt(CURLOPT_URL, url.data());
  setopt(CURLOPT_WRITEFUNCTION, &Request::writeData);
  setopt(CURLOPT_WRITEDATA, this);
  setopt(CURLOPT_PRIVATE, this);
  // set useragent
  curl_multi_add_handle(owner_->getCurlm(), curl_);
}

Request::~Request()
{
  assert(channel_->isNoneEvent());
  curl_multi_remove_handle(owner_->getCurlm(), curl_);
  curl_easy_cleanup(curl_);
}

template<typename OPT>
int Request::setopt(OPT opt, long p)
{
  return curl_easy_setopt(curl_, opt, p);
}

template<typename OPT>
int Request::setopt(OPT opt, const char* p)
{
  return curl_easy_setopt(curl_, opt, p);
}

template<typename OPT>
int Request::setopt(OPT opt, void* p)
{
  return curl_easy_setopt(curl_, opt, p);
}

template<typename OPT>
int Request::setopt(OPT opt, size_t (*p)(char*, size_t , size_t , void*))
{
  return curl_easy_setopt(curl_, opt, p);
}

Channel* Request::setChannel(int fd)
{
  assert(channel_.get() == NULL);
  channel_.reset(new Channel(owner_->getLoop(), fd));
  channel_->tie(shared_from_this());
  return get_pointer(channel_);
}

void Request::done(int code)
{
  if (doneCb_)
  {
    doneCb_(curl_, code);
  }
  owner_->getLoop()->removeChannel(get_pointer(channel_));
}

void Request::dataCallback(const char* buffer, int len)
{
  if (dataCb_)
  {
    dataCb_(buffer, len);
  }
}

size_t Request::writeData(char* buffer, size_t size, size_t nmemb, void* userp)
{
  assert(size == 1);
  Request* req = static_cast<Request*>(userp);
  req->dataCallback(buffer, static_cast<int>(nmemb));
  return nmemb;
}

// ==================================================================

void Curl::initialize(Option opt)
{
  curl_global_init(opt == kCURLnossl ? CURL_GLOBAL_NOTHING : CURL_GLOBAL_SSL);
}

int Curl::socketCallback(CURL* c, int fd, int what, void* userp, void* socketp)
{
  Curl* curl = static_cast<Curl*>(userp);
  const char *whatstr[]={ "none", "IN", "OUT", "INOUT", "REMOVE" };
  LOG_DEBUG << "Curl::socketCallback [" << curl << "] - fd = " << fd
            << " what = " << whatstr[what];
  Request* req = NULL;
  curl_easy_getinfo(c, CURLINFO_PRIVATE, &req);
  assert(req->getCurl() == c);
  if (what == CURL_POLL_REMOVE)
  {
    muduo::net::Channel* ch = static_cast<Channel*>(socketp);
    assert(req->getChannel() == ch);
    ch->disableAll();
  }
  else
  {
    muduo::net::Channel* ch = static_cast<Channel*>(socketp);
    if (!ch)
    {
      ch = req->setChannel(fd);
      ch->setReadCallback(boost::bind(&Curl::onRead, curl, fd));
      ch->setWriteCallback(boost::bind(&Curl::onWrite, curl, fd));
      ch->enableReading();
      curl_multi_assign(curl->curlm_, fd, ch);
    }
    assert(req->getChannel() == ch);
    // update
    if (what & CURL_POLL_OUT)
    {
      ch->enableWriting();
    }
    else
    {
      ch->disableWriting();
    }
  }
  return 0;
}

int Curl::timerCallback(CURLM* curlm, long ms, void* userp)
{
  Curl* curl = static_cast<Curl*>(userp);
  curl->loop_->runAfter(static_cast<int>(ms)/1000.0, boost::bind(&Curl::onTimer, curl));
  return 0;
}

Curl::Curl(EventLoop* loop)
  : loop_(loop),
    curlm_(CHECK_NOTNULL(curl_multi_init())),
    runningHandles_(0),
    prevRunningHandles_(0)
{
  curl_multi_setopt(curlm_, CURLMOPT_SOCKETFUNCTION, &Curl::socketCallback);
  curl_multi_setopt(curlm_, CURLMOPT_SOCKETDATA, this);
  curl_multi_setopt(curlm_, CURLMOPT_TIMERFUNCTION, &Curl::timerCallback);
  curl_multi_setopt(curlm_, CURLMOPT_TIMERDATA, this);
}

Curl::~Curl()
{
  curl_multi_cleanup(curlm_);
}

RequestPtr Curl::getUrl(muduo::StringPiece url)
{
  RequestPtr req(new Request(this, url));
  return req;
}

void Curl::onTimer()
{
  CURLMcode rc = CURLM_OK;
  do {
    rc = curl_multi_socket_action(curlm_, CURL_SOCKET_TIMEOUT, 0, &runningHandles_);
    LOG_TRACE << rc << " " << runningHandles_;
  } while (rc == CURLM_CALL_MULTI_PERFORM);
  checkFinish();
}

void Curl::onRead(int fd)
{
  CURLMcode rc = CURLM_OK;
  do {
    rc = curl_multi_socket_action(curlm_, fd, CURL_POLL_IN, &runningHandles_);
    LOG_TRACE << fd << " " << rc << " " << runningHandles_;
  } while (rc == CURLM_CALL_MULTI_PERFORM);
  checkFinish();
}

void Curl::onWrite(int fd)
{
  CURLMcode rc = CURLM_OK;
  do {
    rc = curl_multi_socket_action(curlm_, fd, CURL_POLL_OUT, &runningHandles_);
    LOG_TRACE << fd << " " << rc << " " << runningHandles_;
  } while (rc == CURLM_CALL_MULTI_PERFORM);
  checkFinish();
}

void Curl::checkFinish()
{
  if (prevRunningHandles_ > runningHandles_ || runningHandles_ == 0)
  {
    CURLMsg* msg = NULL;
    int left = 0;
    while ( (msg = curl_multi_info_read(curlm_, &left)) != NULL)
    {
      if (msg->msg == CURLMSG_DONE)
      {
        CURL* c = msg->easy_handle;
        CURLcode res = msg->data.result;
        Request* req = NULL;
        curl_easy_getinfo(c, CURLINFO_PRIVATE, &req);
        assert(req->getCurl() == c);
        req->done(res);
      }
    }
  }
  prevRunningHandles_ = runningHandles_;
}
