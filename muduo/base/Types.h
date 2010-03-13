#ifndef MUDUO_BASE_TYPES_H
#define MUDUO_BASE_TYPES_H

#include <stdint.h>
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>

///
/// The most common stuffs.
///
namespace muduo
{

// typedef __gnu_cxx::__versa_string<char, std::char_traits<char>, std::allocator<char> > string;
typedef __gnu_cxx::__sso_string string;

}

#endif
