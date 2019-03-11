// Concurrent downloading one file from HTTP

#include "examples/curl/Curl.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include <stdio.h>
#include <sstream>

using namespace muduo;
using namespace muduo::net;

typedef std::shared_ptr<FILE> FilePtr;

template<int N>
bool startWith(const string& str, const char (&prefix)[N])
{
  return str.size() >= N-1 && std::equal(prefix, prefix+N-1, str.begin());
}

class Piece : noncopyable
{
 public:
  Piece(const curl::RequestPtr& req,
        const FilePtr& out,
        const muduo::string& range,
        std::function<void()> done)
    : req_(req),
      out_(out),
      range_(range),
      doneCb_(std::move(done))
  {
    LOG_INFO << "range: " << range;
    req->setRange(range);
    req_->setDataCallback(
        std::bind(&Piece::onData, this, _1, _2));
    req_->setDoneCallback(
        std::bind(&Piece::onDone, this, _1, _2));
  }
 private:
  void onData(const char* data, int len)
  {
    ::fwrite(data, 1, len, get_pointer(out_));
  }

  void onDone(curl::Request* c, int code)
  {
    LOG_INFO << "[" << range_ << "] is done";
    req_.reset();
    out_.reset();
    doneCb_();
  }

  curl::RequestPtr req_;
  FilePtr out_;
  muduo::string range_;
  std::function<void()> doneCb_;
};

class Downloader : noncopyable
{
 public:
  Downloader(EventLoop* loop, const string& url)
    : loop_(loop),
      curl_(loop_),
      url_(url),
      req_(curl_.getUrl(url_)),
      found_(false),
      acceptRanges_(false),
      length_(0),
      pieces_(kConcurrent),
      concurrent_(0)
  {
    req_->setHeaderCallback(
        std::bind(&Downloader::onHeader, this, _1, _2));
    req_->setDoneCallback(
        std::bind(&Downloader::onHeaderDone, this, _1, _2));
    req_->headerOnly();
  }

 private:
  void onHeader(const char* data, int len)
  {
    string line(data, len);
    if (startWith(line, "HTTP/1.1 200") || startWith(line, "HTTP/1.0 200"))
    {
      found_ = true;
    }
    if (line == "Accept-Ranges: bytes\r\n")
    {
      acceptRanges_ = true;
      LOG_DEBUG << "Accept-Ranges";
    }
    else if (startWith(line, "Content-Length:"))
    {
      length_ = atoll(line.c_str() + strlen("Content-Length:"));
      LOG_INFO << "Content-Length: " << length_;
    }
  }

  void onHeaderDone(curl::Request* c, int code)
  {
    LOG_DEBUG << code;
    if (acceptRanges_ && length_ >= kConcurrent * 4096)
    {
      LOG_INFO << "Downloading with " << kConcurrent << " connections";
      concurrent_ = kConcurrent;
      concurrentDownload();
    }
    else if (found_)
    {
      LOG_WARN << "Single connection download";
      FILE* fp = ::fopen("output", "wb");
      if (fp)
      {
        FilePtr(fp, ::fclose).swap(out_);
        req_.reset();
        req2_ = curl_.getUrl(url_);
        req2_->setDataCallback(
            std::bind(&Downloader::onData, this, _1, _2));
        req2_->setDoneCallback(
            std::bind(&Downloader::onDownloadDone, this));
        concurrent_ = 1;
      }
      else
      {
        LOG_ERROR << "Can not create output file";
        loop_->quit();
      }
    }
    else
    {
      LOG_ERROR << "File not found";
      loop_->quit();
    }
  }

  void concurrentDownload()
  {
    const int64_t pieceLen = length_ / kConcurrent;
    for (int i = 0; i < kConcurrent; ++i)
    {
      char buf[256];
      snprintf(buf, sizeof buf, "output-%05d-of-%05d", i, kConcurrent);
      FILE* fp = ::fopen(buf, "wb");
      if (fp)
      {
        FilePtr out(fp, ::fclose);
        curl::RequestPtr req = curl_.getUrl(url_);

        std::ostringstream range;
        if (i < kConcurrent - 1)
        {
          range << i * pieceLen << "-" << (i+1) * pieceLen - 1;
        }
        else
        {
          range << i * pieceLen << "-" << length_ - 1;
        }
        pieces_[i].reset(new Piece(req,
                                   out,
                                   range.str(),
                                   std::bind(&Downloader::onDownloadDone, this)));
      }
      else
      {
        LOG_ERROR << "Can not create output file: " << buf;
        loop_->quit();
      }
    }
  }

  void onData(const char* data, int len)
  {
    ::fwrite(data, 1, len, get_pointer(out_));
  }

  void onDownloadDone()
  {
    if (--concurrent_ <= 0)
    {
      loop_->quit();
    }
  }

  EventLoop* loop_;
  curl::Curl curl_;
  string url_;
  curl::RequestPtr req_;
  curl::RequestPtr req2_;
  bool found_;
  bool acceptRanges_;
  int64_t length_;
  FilePtr out_;
  std::vector<std::unique_ptr<Piece>> pieces_;
  int concurrent_;

  const static int kConcurrent = 4;
};

int main(int argc, char* argv[])
{
  EventLoop loop;
  curl::Curl::initialize(curl::Curl::kCURLssl);
  string url = argc > 1 ? argv[1] : "https://chenshuo-public.s3.amazonaws.com/pdf/allinone.pdf";
  Downloader d(&loop, url);
  loop.loop();
}
