#include "MemcacheServer.h"
#include <muduo/net/EventLoop.h>
#include <muduo/net/inspect/ProcessInspector.h>

#include <stdio.h>
#ifdef HAVE_TCMALLOC
#include <google/heap-profiler.h>
#include <google/malloc_extension.h>
#endif

using namespace muduo::net;

int main(int argc, char* argv[])
{
#ifdef HAVE_TCMALLOC
  MallocExtension::Initialize();
#endif
  int items = argc > 1 ? atoi(argv[1]) : 10000;
  int keylen = argc > 2 ? atoi(argv[2]) : 10;
  int valuelen = argc > 3 ? atoi(argv[3]) : 100;
  EventLoop loop;
  MemcacheServer::Options options;
  MemcacheServer server(&loop, options);

  printf("sizeof(Item) = %zd\npid = %d\nitems = %d\nkeylen = %d\nvaluelen = %d\n",
         sizeof(Item), getpid(), items, keylen, valuelen);
  char key[256] = { 0 };
  string value;
  for (int i = 0; i < items; ++i)
  {
    snprintf(key, sizeof key, "%0*d", keylen, i);
    value.assign(valuelen, "0123456789"[i % 10]);
    ItemPtr item(Item::makeItem(key, 0, 0, valuelen+2, 1));
    item->append(value.data(), value.size());
    item->append("\r\n", 2);
    assert(item->endsWithCRLF());
    bool exists = false;
    bool stored = server.storeItem(item, Item::kAdd, &exists);
    assert(stored); (void) stored;
    assert(!exists);
  }
  Inspector::ArgList arg;
  printf("==========\n%s\n",
         ProcessInspector::overview(HttpRequest::kGet, arg).c_str());
  // TODO: print bytes per item, overhead percent
  fflush(stdout);
#ifdef HAVE_TCMALLOC
  char buf[8192];
  MallocExtension::instance()->GetStats(buf, sizeof buf);
  printf("%s\n", buf);
  HeapProfilerDump("end");

/*
  // only works for tcmalloc_debug
  int blocks = 0;
  size_t total = 0;
  int histogram[kMallocHistogramSize] = { 0, };
  MallocExtension::instance()->MallocMemoryStats(&blocks, &total, histogram);
  printf("==========\nblocks = %d\ntotal = %zd\n", blocks, total);
  for (int i = 0; i < kMallocHistogramSize; ++i)
  {
    printf("%d = %d\n", i, histogram[i]);
  }
*/
#endif
}
