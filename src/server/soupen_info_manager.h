#ifndef SOUPEN_INFO_MANAGER_H_
#define SOUPEN_INFO_MANAGER_H_
#include "../base/soupen_define.h"
namespace soupen_server
{
  class SoupenServerInfoManager
  {
  public:
    //inc can be negative
    SOUPEN_MUST_INLINE static void update_total_memory_used(int64_t inc)
    {
      memory_used_ += inc;
    }
    SOUPEN_MUST_INLINE static int64_t get_total_memory_used()
    {
      return memory_used_;
    }
    SOUPEN_MUST_INLINE static void set_max_memory(int64_t value)
    {
      max_memory_ = value;
    }
    SOUPEN_MUST_INLINE static int64_t get_max_memory()
    {
      return max_memory_;
    }
    SOUPEN_MUST_INLINE static int set_current_db_id(int64_t id)
    {
      int ret = SOUPEN_SUCCESS;
      if (SOUPEN_UNLIKELY(id < 0 || id >= MAX_DB_NUM)) {
        ret = SOUPEN_ERROR_INVALID_ARGUMENT;
      } else {
        current_db_id_ = id;
      }
      return ret;
    }
    SOUPEN_MUST_INLINE static int64_t get_db_id()
    {
      return current_db_id_;
    }
    SOUPEN_MUST_INLINE static bool is_no_more_memory()
    {
      return memory_used_ >= max_memory_;
    }
  private:
    static int64_t memory_used_;
    static int64_t max_memory_;
    static int64_t current_db_id_;
  };
}




#endif /* SOUPEN_INFO_MANAGER_H_ */
