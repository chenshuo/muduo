#include <muduo/base/TimeZone.h>
#include <muduo/base/Types.h>
#include <vector>

namespace
{

struct Transition
{
};

struct Localtime
{
};

}

using namespace muduo;
using namespace std;

struct TimeZone::Data
{
  vector<Transition> transitions_;
  vector<Localtime> localtimes_;
  vector<string> names_;
};

namespace
{
  bool readTimeZoneFile(const char* zonefile, struct TimeZone::Data* data)
  {
    return true;
  }
}


TimeZone::TimeZone(const char* zonefile)
  : data_(new TimeZone::Data)
{
  if (!readTimeZoneFile(zonefile, data_.get()))
  {
    data_.reset();
  }
}

