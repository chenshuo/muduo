#ifndef MUDUO_LOG_LOGGING_H
#define MUDUO_LOG_LOGGING_H

#include <ostream>
#include <boost/scoped_ptr.hpp>

namespace muduo
{

class LoggerImpl;
class Logger
{
 public:
  enum LogLevel
  {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  Logger(const char* file, int line);
  Logger(const char* file, int line, LogLevel level);
  Logger(const char* file, int line, LogLevel level, const char* func);
  Logger(const char* file, int line, bool toAbort);
  ~Logger();

  std::ostream& stream();

 private:
  boost::scoped_ptr<LoggerImpl> impl_;
};

#define LOG_TRACE Logger(__FILE__, __LINE__, Logger::TRACE, __PRETTY_FUNCTION__).stream()
#define LOG_DEBUG Logger(__FILE__, __LINE__, Logger::DEBUG, __PRETTY_FUNCTION__).stream()
#define LOG_INFO Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

}

#endif  // MUDUO_LOG_LOGGING_H
