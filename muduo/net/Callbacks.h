// Copyright 2010 Shuo Chen (chenshuo at chenshuo dot com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MUDUO_NET_CALLBACKS_H
#define MUDUO_NET_CALLBACKS_H

#include <boost/function.hpp>

namespace muduo
{
namespace net
{

// All client visible callbacks go here.

class TcpConnection;
typedef boost::function<void()> TimerCallback;
typedef boost::function<void (TcpConnection*)> ConnectionCallback;
typedef boost::function<void (TcpConnection*, const void*, ssize_t len)> ReadCallback;

}
}

#endif  // MUDUO_NET_CALLBACKS_H
