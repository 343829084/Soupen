#include "../server/soupen_order.h"
#include "../server/soupen_epoll.h"
using namespace soupen_server;

int main(int argc, char *argv[]) {

  SoupenEpoll epoll;
  int ret = init_order_funcs();
  if (SOUPEN_FAILED) {
    exit(1);
  }
  ret = set_order_routine();
  if (SOUPEN_FAILED) {
    exit(1);
  }
  ret = epoll.init();
  if (SOUPEN_FAILED) {
    exit(1);
  }
  epoll.work();
  return 0;
}
