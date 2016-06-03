#ifndef SOUPEN_LOG_V2_H_
#define SOUPEN_LOG_V2_H_
#include "../lib/soupen_time.h"
#include <fstream>
namespace soupen_lib
{

  class SoupenLogV2
  {
  public:
    SoupenLogV2()
    {
      last_log_time_ = SoupenTime::get_curr_datetime();
      set_log_name();
      os_.open(buffer_);
      if (!os_.is_open()) {
        exit(0);
      }
    }
    ~SoupenLogV2()
    {
      if (os_.is_open()) {
        os_.close();
      }
    }
    void set_log_name()
    {
      const char *datetime = SoupenTime::get_curr_datetime("%Y%m%d%H%M%S");
      strcpy(buffer_, datetime);
      strcat(buffer_, ".log");
    }
    inline static SoupenLogV2& get_logger()
    {
      static SoupenLogV2 logger;
      return logger;
    }
    inline bool is_create_new_log_file(int64_t last_time, int64_t current_time)
    {
      return current_time - last_time > INTERVAL_TO_CREATE_NEW_LOG_IN_SECOND;
    }
    inline std::fstream &get_os()
    {
      int64_t curr_time = SoupenTime::get_curr_datetime();
      if (is_create_new_log_file(last_log_time_, curr_time)) {
        os_.close();
        set_log_name();
        os_.open(buffer_);
        if (!os_.is_open()) {
          exit(0);
        }
        last_log_time_ = curr_time;
      }
      return os_;
    }
  private:
    char buffer_[64];
    int64_t last_log_time_;
    std::fstream os_;
    static const int64_t INTERVAL_TO_CREATE_NEW_LOG_IN_SECOND = 1 * 60 * 60;
  };

#define P(x) ","<<#x<<" = "<<x
#define log1(log_info) \
  os<<log_info<<std::endl;
#define log2(log_info, log_content1) \
  os<<log_info<<log_content1<<std::endl;
#define log3(log_info, log_content1, log_content2) \
  os<<log_info<<log_content1<<log_content2<<std::endl;
#define log4(log_info, log_content1, log_content2, log_content3) \
  os<<log_info<<log_content1<<log_content2<<log_content3<<std::endl;
#define log5(log_info, log_content1, log_content2, log_content3, log_content4) \
  os<<log_info<<log_content1<<log_content2<<log_content3<<log_content4<<std::endl;
#define log6(log_info, log_content1, log_content2, log_content3, log_content4, log_content5) \
  os<<log_info<<log_content1<<log_content2<<log_content3<<log_content4<<log_content5<<std::endl;
#define log7(log_info, log_content1, log_content2, log_content3, log_content4, log_content5, log_content6) \
  os<<log_info<<log_content1<<log_content2<<log_content3<<log_content4<<log_content5<<log_content6<<std::endl;

#define GET_LOG_FUN_NAME(_0, _1, _2, _3, _4, _5, _6, _7, FUNC, ...) FUNC
#define Soupen_LOG(...) \
  std::fstream &os = SoupenLogV2::get_logger().get_os();\
  GET_LOG_FUN_NAME(_0, ##__VA_ARGS__, log7, log6, log5, log4, log3, log2, log1, log0)(__VA_ARGS__)

}
#endif /*SOUPEN_LOG_V2_H_*/
