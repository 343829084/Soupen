#include "../lib/soupen_math.h"
#include <algorithm>
using namespace std;
namespace soupen_lib
{
  int SoupenRandomGenerator::init(int32_t init_state)
  {
    state_ = init_state > 0 ? init_state : init_state + M;
    state_ = state_ ? state_ : 1;
    return SOUPEN_SUCCESS;
  }
  int32_t SoupenRandomGenerator::random()
  {
    int32_t tmp = A * (state_ % Q) - R * (state_ / Q);
    state_ = tmp >= 0 ? tmp : tmp + M;
    return state_;
  }
  int32_t SoupenRandomGenerator::operator() ()
  {
    return random();
  }

  int64_t SoupenCaster::char2int(char *start, char *end)
  {
    int64_t ret = 0;
    char *tmp = start;
    int sign = 1;
    if (*start == '-') {
      sign = -1;
      ++tmp;
    }
    while(tmp != end) {
      ret = ret * 10 + (*tmp - '0');
      ++tmp;
    }
    return ret * sign;
  }
  int64_t SoupenCaster::int2char(char *buffer, int64_t value)
  {
    static const char remainder_offset[2][19] = { { '9', '8', '7', '6', '5', '4', '3', '2', '1', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' },//正数

                                                { '9', '8', '7', '6', '5', '4', '3', '2', '1', '0', '9', '8', '7', '6', '5', '4', '3', '2', '1' } };//负数

    static const bool quotient_offset[2][19] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },//正数

                                               { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };//负数
    char *p = buffer;
    int64_t tmp = value;
    int flag = value < 0;
    const char *digit = remainder_offset[flag] + 9;
    const bool *offset = quotient_offset[flag] + 9;
    do {
      int remainder = tmp % 10;
      *p = digit[remainder];
      tmp = tmp / 10 + offset[remainder];
      ++p;
    } while (tmp);
    if(flag) {
      *p++ = '-';
    }
    std::reverse(buffer, p);
    return p - buffer;
  }
}
