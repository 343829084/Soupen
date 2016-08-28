#ifndef SOUPEN_SERVER_EPOLL_H
#define SOUPEN_SERVER_EPOLL_H
#include "../client/soupen_client.h"
namespace soupen_server
{
  class SoupenEpoll
  {
  public:
    int init();
    int work();
  private:
    int process_new_connections(int epfd);
    int create_client(int epfd, int fd);
    int close_client(int epfd, int fd);
    int set_socket_nonblocking(int sock);
    int read_client(int epfd, int fd);
    int write_client(int epfd, int fd);
    static soupen_client::SoupenClientManager client_manager;
  private:
    int sock_;
    static const int DEFAULT_PORT = 7788;
    static const int BUFF_SIZE = 1024 * 1024;
    static const int EPOLL_MAXEVENTS = 10000;
    static const int EPOLL_TIMEOUT = 5000;
  };
}
#endif /*SOUPEN_SERVER_EPOLL_H*/
