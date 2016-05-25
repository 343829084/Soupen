#include <sys/time.h>
#include "../base/yedis_define.h"
namespace yedis_lib
{
  class YedisTime
  {
  public:
    static void get_curr_datetime(char *p, int64_t *usecond = nullptr)
    {
      if (usecond != nullptr) {
        time_t nowtime = *usecond / 1000000 * 1000000;
        struct tm *nowtm = localtime(&nowtime);
        strftime(p, TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", nowtm);
      } else {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        time_t nowtime = tv.tv_sec;
        struct tm *nowtm = localtime(&nowtime);
        strftime(p, TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", nowtm);
      }
    }
    static int64_t get_curr_datetime()
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      return tv.tv_sec * 1000000 + tv.tv_usec;
    }
    static const int64_t TIME_BUFFER_SIZE = 64;
    static const int64_t USECONDS_PER_HOUR = 3600000000LL;
  };
}
