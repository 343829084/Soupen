#ifndef YEDIS_PETERSON_SPIN_LOCK_H_
#define YEDIS_PETERSON_SPIN_LOCK_H_
#include "../base/yedis_common.h"
namespace yedis_sync
{
  class YedisPetersonSpinLock
  {
  public:
    YedisPetersonSpinLock()
    {
      flags_[0] = false;
      flags_[1] = false;
      turn_ = false;
    }
    bool lock(int thread_id)
    {
      int other = !thread_id;
      flags_[thread_id] = true;
      turn_ = thread_id;
      CPU_BARRIER();
      while(flags_[other] && turn_ == thread_id) {
        CPU_RELAX();//spin
      }
      //get lock successfully
      return true;
    }
    bool unlock(int thread_id)
    {
      flags_[thread_id] = false;
      return true;
    }
  private:
    bool flags_[2];
    bool turn_;
  };

  class YedisPetersonSpinLockGuard
  {
  public:
    YedisPetersonSpinLockGuard(YedisPetersonSpinLock &lock, uint16_t thread_id):lock_(lock),thread_id_(thread_id)
    {
      lock_.lock(thread_id_);
    }
    ~YedisPetersonSpinLockGuard()
    {
      lock_.unlock(thread_id_);
    }
  private:
    YedisPetersonSpinLock &lock_;
    uint16_t thread_id_;
  };
}
#endif
