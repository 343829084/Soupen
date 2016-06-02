#ifndef YEDIS_PETERSON_SPIN_LOCK_H_
#define YEDIS_PETERSON_SPIN_LOCK_H_
#include "../base/soupen_common.h"
namespace soupen_sync
{
  class SoupenTicketSpinLock
  {
  public:
    SoupenTicketSpinLock()
    {
      next_id_ = 0;
      service_id_ = 0;
    }
    bool lock()
    {
      uint64_t my_id = FETCH_AND_ADD(&next_id_, 1);
      CPU_BARRIER();
      while(my_id != ACCESS_ONCE(service_id_)) {}
      return true;
    }
    void unlock()
    {
      INC_ATOMIC(&service_id_, 1);
    }
  private:
    uint64_t next_id_;
    uint64_t service_id_;
  };

  class SoupenTicketSpinLockGuard
  {
  public:
    SoupenTicketSpinLockGuard(SoupenTicketSpinLock &lock):lock_(lock)
    {
      lock_.lock();
    }
    ~SoupenTicketSpinLockGuard()
    {
      lock_.unlock();
    }
  private:
    SoupenTicketSpinLock &lock_;
  };
}
#endif
