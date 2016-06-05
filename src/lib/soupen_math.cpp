#include "../lib/soupen_math.h"
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
}
