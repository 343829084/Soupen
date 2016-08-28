#ifndef SOUPEN_MATH_H_
#define SOUPEN_MATH_H_
#include "../base/soupen_define.h"
namespace soupen_lib
{
  class SoupenRandomGenerator
  {
  public:
    int init(int32_t init_state = 1);
    int32_t random();
    int32_t operator() ();
  private:
    int32_t state_;
    static const int32_t A = 48271;
    static const int32_t M = 2147483647;
    static const int32_t Q = M / A;
    static const int32_t R = M % A;
  };
  class SoupenCaster
  {
  public:
    static int64_t char2int(char *start, char *end);
    static int64_t int2char(char *buffer, int64_t value);
  };
}




#endif /* SOUPEN_MATH_H_ */
