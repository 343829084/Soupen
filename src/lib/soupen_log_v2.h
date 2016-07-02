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
      os_.open(buffer_, std::ios::out);
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
        os_.open(buffer_, std::ios::out);
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

  #define soupen_log1(log_info) \
    os<<datetime<<"["<<__func__<<"]"<< __FILE__<< ":"<<__LINE__<<"<****>"<<log_info<<std::endl;
  #define soupen_log2(log_info, log_content1) \
    os<<datetime<<"["<<__func__<<"]"<< __FILE__<< ":"<<__LINE__<<"<****>"<<log_info<<log_content1<<std::endl;
  #define soupen_log3(log_info, log_content1, log_content2) \
    os<<datetime<<"["<<__func__<<"]"<< __FILE__<< ":"<<__LINE__<<"<****>"<<log_info<<log_content1<<log_content2<<std::endl;
  #define soupen_log4(log_info, log_content1, log_content2, log_content3) \
    os<<datetime<<"["<<__func__<<"]"<< __FILE__<< ":"<<__LINE__<<"<****>"<<log_info<<log_content1<<log_content2<<log_content3<<std::endl;
  #define soupen_log5(log_info, log_content1, log_content2, log_content3, log_content4) \
    os<<datetime<<"["<<__func__<<"]"<< __FILE__<< ":"<<__LINE__<<"<****>"<<log_info<<log_content1<<log_content2<<log_content3<<log_content4<<std::endl;
  #define soupen_log6(log_info, log_content1, log_content2, log_content3, log_content4, log_content5) \
    os<<datetime<<"["<<__func__<<"]"<< __FILE__<< ":"<<__LINE__<<"<****>"<<log_info<<log_content1<<log_content2<<log_content3<<log_content4<<log_content5<<std::endl;
  #define soupen_log7(log_info, log_content1, log_content2, log_content3, log_content4, log_content5, log_content6) \
    os<<datetime<<"["<<__func__<<"]"<< __FILE__<< ":"<<__LINE__<<"<****>"<<log_info<<log_content1<<log_content2<<log_content3<<log_content4<<log_content5<<log_content6<<std::endl;

  #define GET_LOG_FUN_NAME(_0, _1, _2, _3, _4, _5, _6, _7, FUNC, ...) FUNC

  #define Soupen_LOG(...) \
  do{\
    std::fstream &os = SoupenLogV2::get_logger().get_os();\
    const char *dt_fmt = "%Y-%m-%d %H:%M:%S";\
    char *datetime = soupen_lib::SoupenTime::get_curr_datetime(dt_fmt);\
    GET_LOG_FUN_NAME(_0, ##__VA_ARGS__, soupen_log7, soupen_log6, soupen_log5, soupen_log4, soupen_log3, soupen_log2, soupen_log1, soupen_log0)(__VA_ARGS__)\
  }while(0);
}
#endif /*SOUPEN_LOG_V2_H_*/
