#include <muduo/base/Logging.h>

#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>

#include <errno.h>                      // for errno
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace muduo
{
class LoggerImpl
{
 public:
  typedef Logger::LogLevel LogLevel;
  LoggerImpl(LogLevel level, int old_errno, const char* file, int line);

  Timestamp time_;
  std::ostringstream stream_;
  LogLevel level_;
  const char* fullname_;
  int line_;
  const char* basename_;
  const char* function_;

  static const char* LogLevelName[];
};
}

using namespace muduo;

const char* LoggerImpl::LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE",
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL",
};

LoggerImpl::LoggerImpl(LogLevel level, int savedErrno, const char* file, int line)
  : time_(Timestamp::now()),
    stream_(),
    level_(level),
    fullname_(file),
    line_(line),
    basename_(NULL),
    function_(NULL)
{
  const char* path_sep_pos = strrchr(fullname_, '/');
  basename_ = (path_sep_pos != NULL) ? path_sep_pos + 1 : fullname_;
  char message_head[512];
  snprintf(message_head, sizeof(message_head), "%s %5d %s:%d %s ",
      time_.toFormattedString().c_str(), CurrentThread::tid(),
      basename_, line_, LogLevelName[level]);
  stream_ << message_head;
  if (savedErrno != 0)
  {
    stream_ << strerror(savedErrno);
  }
}

std::ostream& Logger::stream()
{
  return impl_->stream_;
}

Logger::Logger(const char* file, int line)
  : impl_(new LoggerImpl(INFO, 0, file, line))
{
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
  : impl_(new LoggerImpl(level, 0, file, line))
{
  impl_->stream_ << func << ' ';
}

Logger::Logger(const char* file, int line, LogLevel level)
  : impl_(new LoggerImpl(level, 0, file, line))
{
}

Logger::Logger(const char* file, int line, bool toAbort)
  : impl_(new LoggerImpl(toAbort?FATAL:ERROR, errno, file, line))
{
}

Logger::~Logger()
{
  impl_->stream_ << '\n';
  std::string buf(impl_->stream_.str());
  ssize_t n = ::write(1, buf.data(), buf.size());
  if (impl_->level_ == FATAL)
  {
    abort();
  }
}

