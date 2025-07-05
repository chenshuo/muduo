#pragma once

#include "muduo/base/noncopyable.h"
#include "muduo/net/Buffer.h"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <zlib.h>

namespace muduo
{
namespace net
{

// input is zlib compressed data, output uncompressed data
// FIXME: finish this
class ZlibInputStream : noncopyable
{
 public:
  explicit ZlibInputStream(Buffer* output)
    : output_(output),
      zerror_(Z_OK)
  {
    memZero(&zstream_, sizeof zstream_);
    zerror_ = inflateInit(&zstream_);
  }

  ~ZlibInputStream()
  {
    finish();
  }
  int zlibErrorCode() const { return zerror_; }
  bool write(StringPiece buf)
  {
    if (zerror_ != Z_OK && zerror_ != Z_STREAM_END)
      return false;

    assert(zstream_.next_in == NULL && zstream_.avail_in == 0);
    void* in = const_cast<char*>(buf.data());
    zstream_.next_in = static_cast<Bytef*>(in);
    zstream_.avail_in = buf.size();
    while (zstream_.avail_in > 0 && zerror_ == Z_OK)
    {
      zerror_ = decompress(Z_NO_FLUSH);
    }
    if (zstream_.avail_in == 0)
    {
      assert(static_cast<const void*>(zstream_.next_in) == buf.end());
      zstream_.next_in = NULL;
    }
    return zerror_ == Z_OK || zerror_ == Z_STREAM_END;
  }

  // compress input as much as possible, not guarantee consuming all data.
  bool write(Buffer* input)
  {
    if (zerror_ != Z_OK && zerror_ != Z_STREAM_END)
      return false;

    void* in = const_cast<char*>(input->peek());
    zstream_.next_in = static_cast<Bytef*>(in);
    zstream_.avail_in = static_cast<int>(input->readableBytes());
    if (zstream_.avail_in > 0 && zerror_ == Z_OK)
    {
      zerror_ = decompress(Z_NO_FLUSH);
    }
    input->retrieve(input->readableBytes() - zstream_.avail_in);
    return zerror_ == Z_OK || zerror_ == Z_STREAM_END;
  }
  bool finish()
  {
    if (zerror_ != Z_OK  && zerror_ != Z_STREAM_END) {
      return false;
    }
    zerror_ = inflateEnd(&zstream_);
    bool ok = (zerror_ == Z_OK  || zerror_ == Z_STREAM_END);
    zerror_ = Z_STREAM_END;
    return ok;
  }

 private:
  int decompress(int flush)
  {
    int buffer_size = 1024;
    output_->ensureWritableBytes(buffer_size);
    zstream_.next_out = reinterpret_cast<Bytef*>(output_->beginWrite());
    zstream_.avail_out = static_cast<int>(output_->writableBytes());
    int error = ::inflate(&zstream_, flush);
    output_->hasWritten(output_->writableBytes() - zstream_.avail_out);
    if (output_->writableBytes() == 0 && buffer_size < 65536)
    {
      buffer_size *= 2;
    }
    return error;
  }

  Buffer* output_;
  z_stream zstream_;
  int zerror_;
};

// input is uncompressed data, output zlib compressed data
class ZlibOutputStream : noncopyable
{
 public:
  explicit ZlibOutputStream(Buffer* output)
    : output_(output),
      zerror_(Z_OK),
      bufferSize_(1024)
  {
    memZero(&zstream_, sizeof zstream_);
    zerror_ = deflateInit(&zstream_, Z_DEFAULT_COMPRESSION);
  }

  ~ZlibOutputStream()
  {
    finish();
  }

  // Return last error message or NULL if no error.
  const char* zlibErrorMessage() const { return zstream_.msg; }

  int zlibErrorCode() const { return zerror_; }
  int64_t inputBytes() const { return zstream_.total_in; }
  int64_t outputBytes() const { return zstream_.total_out; }
  int internalOutputBufferSize() const { return bufferSize_; }

  bool write(StringPiece buf)
  {
    if (zerror_ != Z_OK && zerror_ != Z_STREAM_END)
      return false;

    assert(zstream_.next_in == NULL && zstream_.avail_in == 0);
    void* in = const_cast<char*>(buf.data());
    zstream_.next_in = static_cast<Bytef*>(in);
    zstream_.avail_in = buf.size();
    while (zstream_.avail_in > 0 && zerror_ == Z_OK)
    {
      zerror_ = compress(Z_NO_FLUSH);
    }
    if (zstream_.avail_in == 0)
    {
      assert(static_cast<const void*>(zstream_.next_in) == buf.end());
      zstream_.next_in = NULL;
    }
    return zerror_ == Z_OK || zerror_ == Z_STREAM_END;
  }

  // compress input as much as possible, not guarantee consuming all data.
  bool write(Buffer* input)
  {
    if (zerror_ != Z_OK && zerror_ != Z_STREAM_END)
      return false;

    void* in = const_cast<char*>(input->peek());
    zstream_.next_in = static_cast<Bytef*>(in);
    zstream_.avail_in = static_cast<int>(input->readableBytes());
    if (zstream_.avail_in > 0 && zerror_ == Z_OK)
    {
      zerror_ = compress(Z_NO_FLUSH);
    }
    input->retrieve(input->readableBytes() - zstream_.avail_in);
    return zerror_ == Z_OK || zerror_ == Z_STREAM_END;
  }

  bool finish()
  {
    if (zerror_ != Z_OK && zerror_ != Z_STREAM_END)
      return false;

    while (zerror_ == Z_OK)
    {
      zerror_ = compress(Z_FINISH);
    }
    zerror_ = deflateEnd(&zstream_);
    bool ok = zerror_ == Z_OK;
    zerror_ = Z_STREAM_END;
    return ok;
  }

 private:
  int compress(int flush)
  {
    output_->ensureWritableBytes(bufferSize_);
    zstream_.next_out = reinterpret_cast<Bytef*>(output_->beginWrite());
    zstream_.avail_out = static_cast<int>(output_->writableBytes());
    int error = ::deflate(&zstream_, flush);
    output_->hasWritten(output_->writableBytes() - zstream_.avail_out);
    if (output_->writableBytes() == 0 && bufferSize_ < 65536)
    {
      bufferSize_ *= 2;
    }
    return error;
  }

  Buffer* output_;
  z_stream zstream_;
  int zerror_;
  int bufferSize_;
};

}  // namespace net
}  // namespace muduo
