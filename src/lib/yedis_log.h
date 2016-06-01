#include "../lib/yedis_time.h"
#include <stdlib.h>
#include <stdio.h>
namespace yedis_lib
{

  #define WARN "warn"
  #define ERROR "error"
  #define INFO "info"
  #define DEBUG "debug"

  class YedisLog
  {
  public:
    YedisLog()
    {
      fp_ = nullptr;
      log_time_ = YedisTime::get_curr_datetime();
      get_log_name(&log_time_);
      fp_= fopen(buffer_, "a+");
      if (nullptr == fp_) {
        exit(0);
      }
    }
    ~YedisLog()
    {
      if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
      }
    }
    void get_log_name(int64_t *usecond)
    {
      const char *datetime = YedisTime::get_curr_datetime("%Y%m%d%H%M%S");
      strcpy(buffer_, datetime);
      strcat(buffer_, ".log");
    }
    inline static YedisLog& get_logger()
    {
      static YedisLog logger;
      return logger;
    }
    inline bool is_create_new_log_file(int64_t last_time, int64_t current_time)
    {
      return current_time - last_time > YedisTime::INTERVAL_TO_CREATE_NEW_LOG_IN_SECOND;
    }
    inline FILE *get_fp()
    {
      FILE *fp = nullptr;
      int64_t curr_time = YedisTime::get_curr_datetime();
      if (is_create_new_log_file(log_time_, curr_time)) {
        fclose(fp_);
        get_log_name(&curr_time);
        fp_ = fopen(buffer_, "a+");
        if (fp_ == nullptr) {
          exit(0);
        }
        fp = fp_;
        log_time_ = curr_time;
      } else {
        fp = fp_;
      }
      return fp;
    }
  private:
    char buffer_[64];
    int64_t log_time_;
    FILE *fp_;
  };

  #define __LOG__(format, loglevel, ...)  \
    FILE *fp = yedis_lib::YedisLog::get_logger().get_fp(); \
    const char *dt_fmt = "%Y-%m-%d %H:%M:%S";\
    char *datetime = yedis_lib::YedisTime::get_curr_datetime(dt_fmt);\
    fprintf(fp, "%s %-5s [%s] [%s:%d] " format "\n", loglevel, datetime, __func__, __FILE__, __LINE__, ## __VA_ARGS__);

  #define LOG_DEBUG(format, ...) __LOG__(format, DEBUG, ## __VA_ARGS__)
  #define LOG_WARN(format, ...) __LOG__(format, WARN, ## __VA_ARGS__)
  #define LOG_ERROR(format, ...) __LOG__(format, ERROR, ## __VA_ARGS__)
  #define LOG_INFO(format, ...) __LOG__(format, INFO, ## __VA_ARGS__)

}
