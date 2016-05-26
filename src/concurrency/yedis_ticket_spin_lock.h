#ifndef YEDIS_PETERSON_SPIN_LOCK_H_
#define YEDIS_PETERSON_SPIN_LOCK_H_
#include "../base/yedis_common.h"
namespace yedis_sync
{
  class YedisTicketSpinLock
  {
  public:
    YedisTicketSpinLock()
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

  class YedisTicketSpinLockGuard
  {
  public:
    YedisTicketSpinLockGuard(YedisTicketSpinLock &lock):lock_(lock)
    {
      lock_.lock();
    }
    ~YedisTicketSpinLockGuard()
    {
      lock_.unlock();
    }
  private:
    YedisTicketSpinLock &lock_;
  };
}
#endif
