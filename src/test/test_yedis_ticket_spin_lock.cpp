#include "../sync/yedis_ticket_spin_lock.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace yedis_sync;
YedisTicketSpinLock sl;
int c = 0;
void ticket_spin_lock_f()
{
  int counter = 0;
  while(counter++ < 10000000) {
    YedisTicketSpinLockGuard rl(sl);
    ++c;
  }
}
int ticket_spin_lock_main()
{
  while(true) {
    c = 0;
  thread reader1(ticket_spin_lock_f);
  thread reader2(ticket_spin_lock_f);
  thread reader3(ticket_spin_lock_f);
  thread reader4(ticket_spin_lock_f);
  reader1.join();
  reader2.join();
  reader3.join();
  reader4.join();
  if (c != 40000000)
    cout<<c<<endl;
  }
  return 0;
}
