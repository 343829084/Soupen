#include "../sync/soupen_rw_lock.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace soupen_sync;
SoupenRWLock rw_lock;
void f()
{
  int counter = 0;
  while(++counter < 10000000) {
    SoupenReaderLockGuard rl(rw_lock, 1);
  }
}
void g()
{
  int counter = 0;
  while(++counter < 10000000) {
    SoupenWriterLockGuard wl(rw_lock);
  }
}
int main()
{
  thread reader1(f);
  thread reader2(f);
  thread writer(g);
  reader1.join();
  reader2.join();
  writer.join();
  return 0;
}
