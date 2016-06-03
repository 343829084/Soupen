#ifndef SOUPEN_TIME_H_
#define SOUPEN_TIME_H_
#include <time.h>
#include "../base/soupen_define.h"
namespace soupen_lib
{
  class SoupenTime
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
  };
}
#endif /*SOUPEN_TIME_H_*/
