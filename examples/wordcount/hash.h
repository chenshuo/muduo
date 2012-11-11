#ifndef MUDUO_EXAMPLES_WORDCOUNT_HASH_H
#define MUDUO_EXAMPLES_WORDCOUNT_HASH_H

namespace boost
{
std::size_t hash_value(const muduo::string& x);
}

#include <boost/unordered_map.hpp>

namespace boost
{
inline std::size_t hash_value(const muduo::string& x)
{
  return hash_range(x.begin(), x.end());
}
// template <> struct hash<muduo::string>
// {
//   std::size_t operator()(const muduo::string& v) const
//   {
//     return hash_value(v);
//   }
// };
}

typedef boost::unordered_map<muduo::string, int64_t> WordCountMap;

#endif  // MUDUO_EXAMPLES_WORDCOUNT_HASH_H
