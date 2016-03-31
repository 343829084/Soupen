#include "../base/yedis_common.h"
#include "../server/yedis_order.h"
#include "../server/yedis_epoll.h"
#include<iostream>
using namespace std;
using namespace yedis_server;

int main(int argc, char *argv[]) {

  init_order_funcs();
  set_order_routine();
  YedisEpoll epoll;
  epoll.init();
  epoll.work();
  return 0;
}
