#ifndef SOUPEN_LOG_H_
#define SOUPEN_LOG_H_
#include "../lib/soupen_time.h"
#include <stdlib.h>
#include <stdio.h>
namespace soupen_lib
{

  #define WARN "warn"
  #define ERROR "error"
  #define INFO "info"
  #define DEBUG "debug"

  class SoupenLog
  {
  public:
    SoupenLog()
    {
      fp_ = nullptr;
      last_log_time_ = SoupenTime::get_curr_datetime();
      set_log_name();
      fp_= fopen(buffer_, "a+");
      if (nullptr == fp_) {
        exit(0);
      }
    }
    ~SoupenLog()
    {
      if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
      }
    }
    void set_log_name()
    {
      const char *datetime = SoupenTime::get_curr_datetime("%Y%m%d%H%M%S");
      strcpy(buffer_, datetime);
      strcat(buffer_, ".log");
    }
    inline static SoupenLog& get_logger()
    {
      static SoupenLog logger;
      return logger;
    }
    inline bool is_create_new_log_file(int64_t last_time, int64_t current_time)
    {
      return current_time - last_time > INTERVAL_TO_CREATE_NEW_LOG_IN_SECOND;
    }
    inline FILE *get_fp()
    {
      int64_t curr_time = SoupenTime::get_curr_datetime();
      if (is_create_new_log_file(last_log_time_, curr_time)) {
        fclose(fp_);
        set_log_name();
        fp_ = fopen(buffer_, "a+");
        if (fp_ == nullptr) {
          exit(0);
        }
        last_log_time_ = curr_time;
      }
      return fp_;
    }
  private:
    char buffer_[64];
    int64_t last_log_time_;
    FILE *fp_;
    static const int64_t INTERVAL_TO_CREATE_NEW_LOG_IN_SECOND = 1 * 60 * 60;
  };

  #define __LOG__(format, loglevel, ...)  \
    FILE *fp = soupen_lib::SoupenLog::get_logger().get_fp(); \
    const char *dt_fmt = "%Y-%m-%d %H:%M:%S";\
    char *datetime = soupen_lib::SoupenTime::get_curr_datetime(dt_fmt);\
    fprintf(fp, "%s %-5s [%s] [%s:%d] " format "\n", loglevel, datetime, __func__, __FILE__, __LINE__, ## __VA_ARGS__);

  #define LOG_DEBUG(format, ...) __LOG__(format, DEBUG, ## __VA_ARGS__)
  #define LOG_WARN(format, ...) __LOG__(format, WARN, ## __VA_ARGS__)
  #define LOG_ERROR(format, ...) __LOG__(format, ERROR, ## __VA_ARGS__)
  #define LOG_INFO(format, ...) __LOG__(format, INFO, ## __VA_ARGS__)

}
#endif /*SOUPEN_LOG_H_*/
