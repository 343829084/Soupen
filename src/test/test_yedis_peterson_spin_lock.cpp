#include "../sync/yedis_peterson_spin_lock.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace yedis_sync;
YedisPetersonSpinLock sll;
int cccc = 0;
void peterson_spin_lock_f()
{
  int counter = 0;
  while(counter++ < 10000000) {
    YedisPetersonSpinLockGuard rl(sll, 0);
    ++cccc;
  }
}
void peterson_spin_lock_g()
{
  int counter = 0;
  while(counter++ < 10000000) {
    YedisPetersonSpinLockGuard wl(sll, 1);
    ++cccc;
  }
}
int main()
{
  for (int i = 0 ; i <= 1000; ++i) {
    cccc = 0;
      timespec t1, t2;
      //clock_gettime(CLOCK_MONOTONIC, &t1);
    thread reader1(peterson_spin_lock_f);
    thread reader2(peterson_spin_lock_g);
    reader1.join();
    reader2.join();
   // clock_gettime(CLOCK_MONOTONIC, &t2);
    long long tmp = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec;
    cout<<tmp<<endl;
    if (cccc != 20000000) {
      cout<<"bad"<<endl;
      exit(1);
    }
  }
  cout<<"good"<<endl;
  return 0;
}
