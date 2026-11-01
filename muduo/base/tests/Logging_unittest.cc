// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

#include "muduo/base/Logging.h"

#include <algorithm>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

namespace
{
std::vector<muduo::string> records;

void capture(const char* message, int len)
{
  records.emplace_back(message, static_cast<size_t>(len));
}

void check(bool condition)
{
  if (!condition)
  {
    fputs("Logging_unittest: invalid log record boundary\n", stderr);
    abort();
  }
}
}  // namespace

int main()
{
  muduo::Logger::setOutput(capture);
  {
    muduo::Logger(__FILE__, __LINE__).stream()
        << muduo::string(muduo::detail::kSmallBuffer * 2, 'x');
  }
  {
    muduo::Logger(__FILE__, __LINE__).stream() << "next record";
  }

  check(records.size() == 2);
  check(records[0].size() == muduo::detail::kSmallBuffer - 1);
  check(records[0].back() == '\n');
  check(records[0][records[0].size() - 2] == 'x');
  check(records[1].find("next record - ") != muduo::string::npos);
  check(records[1].back() == '\n');
  const muduo::string output = records[0] + records[1];
  check(std::count(output.begin(), output.end(), '\n') == 2);
}
