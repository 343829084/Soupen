#include "../server/yedis_order.h"
#include "../server/yedis_epoll.h"
using namespace yedis_server;

int main(int argc, char *argv[]) {

  YedisEpoll epoll;
  int ret = init_order_funcs();
  if (YEDIS_FAILED) {
    exit(1);
  }
  ret = set_order_routine();
  if (YEDIS_FAILED) {
    exit(1);
  }
  ret = epoll.init();
  if (YEDIS_FAILED) {
    exit(1);
  }
  epoll.work();
  return 0;
}
