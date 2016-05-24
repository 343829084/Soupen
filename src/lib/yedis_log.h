#include <fstream>
#include <iostream>
#include "../lib/yedis_time.h"
namespace yedis_lib
{

#define WARN "warn"
#define ERROR "error"

  class YedisLog
  {
  public:
    YedisLog()
    {
      curr_time_ = YedisTime::get_curr_datetime();
      get_log_name(&curr_time_);
      os_.open(buffer_);
      if (os_.fail()) {
        exit(0);
      }
    }
    ~YedisLog()
    {
      os_.close();
    }
    void get_log_name(int64_t *usecond)
    {
      YedisTime::get_curr_datetime(buffer_, usecond);
      strcat(buffer_, ".log");
    }
    template<typename T>
    void log(const T &value, const char *level_type)
    {
      int64_t curr_time = YedisTime::get_curr_datetime();
      if (curr_time - curr_time_ > YedisTime::USECONDS_PER_HOUR) {
        os_.close();
        get_log_name(&curr_time);
        os_.open(buffer_);
        if (!os_.fail()) {
          os_<<__FILE__<<","<<__LINE__<<"["<<level_type<<"]"<<value<<std::endl;
          curr_time_ = curr_time;
        }
      } else {
        os_<<__FILE__<<","<<__LINE__<<"["<<level_type<<"]"<<value<<std::endl;
      }
    }
  private:
    char buffer_[YedisTime::TIME_BUFFER_SIZE + 10];
    int64_t curr_time_;
    std::fstream os_;
  };

#define LOG_WARN(output) logger.log(output, WARN);

#define LOG_ERROR(output) logger.log(output, ERROR);

}
