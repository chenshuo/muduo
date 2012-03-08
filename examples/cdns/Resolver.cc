#include <examples/cdns/Resolver.h>

#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

#include <ares.h>
#include <netdb.h>
#include <arpa/inet.h>  // inet_ntop

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

using namespace muduo;
using namespace muduo::net;
using namespace cdns;

Resolver::Resolver(EventLoop* loop, Option opt)
  : loop_(loop),
    ctx_(NULL)
{
  static char lookups[] = "b";
  struct ares_options options;
  int optmask = ARES_OPT_FLAGS;
  options.flags = ARES_FLAG_NOCHECKRESP;
  options.flags |= ARES_FLAG_STAYOPEN;
  options.flags |= ARES_FLAG_IGNTC; // UDP only
  optmask |= ARES_OPT_SOCK_STATE_CB;
  options.sock_state_cb = &Resolver::ares_sock_state_callback;
  options.sock_state_cb_data = this;
  if (opt == kDNSonly)
  {
    optmask |= ARES_OPT_LOOKUPS;
    options.lookups = lookups;
  }

  int status = ares_init_options(&ctx_, &options, optmask);
  if (status != ARES_SUCCESS)
  {
    assert(0);
  }
  ares_set_socket_callback(ctx_, &Resolver::ares_sock_create_callback, this);
}

Resolver::~Resolver()
{
  ares_destroy(ctx_);
}

bool Resolver::resolve(const StringPiece& hostname, const Callback& cb)
{
  loop_->assertInLoopThread();
  printf("resolve %s\n", hostname.data());
  QueryData* queryData = new QueryData(this, cb);
  ares_gethostbyname(ctx_, hostname.data(), AF_INET,
      &Resolver::ares_host_callback, queryData);
  struct timeval tv;
  struct timeval* tvp = ares_timeout(ctx_, NULL, &tv);
  // FIXME timer
  printf("timeout %ld.%06ld\n", tvp->tv_sec, tv.tv_usec);
  return queryData != NULL;
}

void Resolver::onRead(int sockfd, Timestamp t)
{
  printf("onRead %d\n", sockfd);
  ares_process_fd(ctx_, sockfd, ARES_SOCKET_BAD);
}

void Resolver::onQueryResult(int status, struct hostent* result, const Callback& callback)
{
  printf("onQueryResult %p %d\n", result, status);
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  if (result)
  {
    printf("h_name %s\n", result->h_name);
    for (char** alias = result->h_aliases; *alias != NULL; ++alias)
    {
      printf("alias: %s\n", *alias);
    }
    // printf("ttl %d\n", ttl);
    // printf("h_length %d\n", result->h_length);
    for (char** haddr = result->h_addr_list; *haddr != NULL; ++haddr)
    {
      char buf[32];
      inet_ntop(AF_INET, *haddr, buf, sizeof buf);
      printf("  %s\n", buf);
    }
    addr.sin_addr = *reinterpret_cast<in_addr*>(result->h_addr);
  }
  InetAddress inet(addr);
  callback(inet);
}

void Resolver::onSockCreate(int sockfd, int type)
{
  loop_->assertInLoopThread();
  assert(channels_.find(sockfd) == channels_.end());
  Channel* channel = new Channel(loop_, sockfd);
  channel->setReadCallback(boost::bind(&Resolver::onRead, this, sockfd, _1));
  channel->enableReading();
  channels_.insert(sockfd, channel);
}

void Resolver::onSockStateChange(int sockfd, bool read, bool write)
{
  loop_->assertInLoopThread();
  ChannelList::iterator it = channels_.find(sockfd);
  assert(it != channels_.end());
  if (read)
  {
    // update
    // if (write) { } else { }
  }
  else
  {
    // remove
    it->second->disableAll();
    loop_->removeChannel(it->second);
    channels_.erase(it);
  }
}

void Resolver::ares_host_callback(void* data, int status, int timeouts, struct hostent* hostent)
{
  QueryData* query = static_cast<QueryData*>(data);

  printf("ares_host_callback %p\n", query);
  query->owner->onQueryResult(status, hostent, query->callback);
  delete query;
}

int Resolver::ares_sock_create_callback(int sockfd, int type, void* data)
{
  printf("fd %d type %d\n", sockfd, type);
  static_cast<Resolver*>(data)->onSockCreate(sockfd, type);
  return 0;
}

void Resolver::ares_sock_state_callback(void* data, int sockfd, int read, int write)
{
  printf("sock_state %d read %d write %d\n", sockfd, read ,write);
  static_cast<Resolver*>(data)->onSockStateChange(sockfd, read, write);
}

