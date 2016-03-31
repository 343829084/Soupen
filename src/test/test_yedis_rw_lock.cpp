#include "../sync/yedis_rw_lock.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace yedis_sync;
YedisRWLock rw_lock;
void f()
{
  int counter = 0;
  while(++counter < 10000000) {
    YedisReaderLockGuard rl(rw_lock, 1);
  }
}
void g()
{
  int counter = 0;
  while(++counter < 10000000) {
    YedisWriterLockGuard wl(rw_lock);
  }
}
int maintt()
{
  thread reader1(f);
  thread reader2(f);
  thread writer(g);
  reader1.join();
  reader2.join();
  writer.join();
  return 0;
}
