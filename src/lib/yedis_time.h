#include <time.h>
#include "../base/yedis_define.h"
namespace yedis_lib
{
  class YedisTime
  {
  public:
    static char *get_curr_datetime(const char *format)
    {
      time_t tmp_time;
      time(&tmp_time);
      struct tm* ti = localtime(&tmp_time);
      static char buffer[64];
      strftime(buffer, sizeof(buffer), format, ti);
      return buffer;
    }
    static int64_t get_curr_datetime()
    {
      time_t tmp_time;
      time(&tmp_time);
      return tmp_time;
    }
    static const int64_t INTERVAL_TO_CREATE_NEW_LOG_IN_SECOND = 10;
  };
}
