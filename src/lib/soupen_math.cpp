#include "../lib/soupen_math.h"
namespace soupen_lib
{
  SoupenRandomGenerator::SoupenRandomGenerator(int32_t init_state)
  {
    state_ = init_state > 0 ? init_state : init_state + M;
    state_ = state_ ? state_ : 1;
  }
  int32_t SoupenRandomGenerator::random()
  {
    int32_t tmp = A * (state_ % Q) - R * (state_ / Q);
    if (tmp >= 0) {
      state_ = tmp;
    } else {
      state_ = tmp + M;
    }
    return state_;
  }
  int SoupenRandomGenerator::operator() ()
  {
    return random();
  }
}
