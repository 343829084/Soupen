#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include "../base/soupen_common.h"
#include "../server/soupen_epoll.h"
#include "../server/soupen_info_manager.h"
#include "../server/soupen_order.h"
namespace soupen_server
{
  int SoupenEpoll::init()
  {
    int ret = SOUPEN_SUCCESS;
    int yes = 1;

    if ((sock_ = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
      ret = SOUPEN_ERROR_UNEXPECTED;
    } else if(setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
      ret = SOUPEN_ERROR_UNEXPECTED;
    } else if(SOUPEN_SUCCESS != (ret = set_socket_nonblocking(sock_))) {

    } else {

      struct sockaddr_in bind_addr;
      memset(&bind_addr, 0, sizeof(bind_addr));
      bind_addr.sin_family = AF_INET;
      bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      bind_addr.sin_port = htons(DEFAULT_PORT);

      if (bind(sock_, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) == -1) {
        ret = SOUPEN_ERROR_UNEXPECTED;
      }
    }
    return ret;
  }
  int SoupenEpoll::set_socket_nonblocking(int sock_)
  {
    int ret = SOUPEN_SUCCESS;
    int flags = fcntl(sock_, F_GETFL, 0);
    if (flags < 0) {
      ret = SOUPEN_ERROR_UNEXPECTED;
    } else if (fcntl(sock_, F_SETFL, flags | O_NONBLOCK) < 0) {
      ret = SOUPEN_ERROR_UNEXPECTED;
    }
    return ret;
  }
  int SoupenEpoll::work()
  {
    //listen
    int ret = SOUPEN_SUCCESS;
    int epfd;
    char response[12 * 1024];
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sock_;
    if (listen(sock_, 5) == -1) {
      ret = SOUPEN_ERROR_UNEXPECTED;
    } else if ((epfd = epoll_create(1)) == -1) {
      ret = SOUPEN_ERROR_UNEXPECTED;
    } else if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock_, &event) == -1) {
      ret = SOUPEN_ERROR_UNEXPECTED;
    } else {
      struct epoll_event events[EPOLL_MAXEVENTS];
      memset(events, 0, sizeof(events));
      int conn_sock;
      struct sockaddr_in client_addr;
      socklen_t client_addr_len;
      char client_ip_str[INET_ADDRSTRLEN];
      int res;
      int i;
      char buffer[BUFF_SIZE];
      int recv_size;
      while (1) {
        res = epoll_wait(epfd, events, EPOLL_MAXEVENTS, EPOLL_TIMEOUT);
        if (res < 0) {
          Soupen_LOG("epoll_wait error", P(res), P(stderr));
          exit(EXIT_FAILURE);
        } else if (res == 0) {
          fprintf(stderr, "memory used right now: %lld Bytes\n", SoupenServerInfoManager::get_total_memory_used());
          continue;
        }
        //res > 0
        for (i = 0; i < res; i++) {
          if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
            Soupen_LOG("epoll error", P(events[i].events), P(stderr));
            close(events[i].data.fd);
            continue;
          }

          if (events[i].data.fd == sock_) { //new connection
            while (1) {
              client_addr_len = sizeof(client_addr);
              conn_sock = accept(sock_, (struct sockaddr *) &client_addr, &client_addr_len);
              if (conn_sock == -1) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                  break;
                } else {
                  Soupen_LOG("accept failed", P(errno));
                  exit(EXIT_FAILURE);
                }
              }
              if (!inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, sizeof(client_ip_str))) {
                Soupen_LOG("inet_ntop failed", P(errno));
                exit(EXIT_FAILURE);
              }
              printf("accept a client from: %s\n", client_ip_str);
              set_socket_nonblocking(conn_sock);
              event.events = EPOLLIN;
              event.data.fd = conn_sock;
              if (epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock, &event) == -1) {
                Soupen_LOG("epoll adds event failed", P(errno));
                exit(EXIT_FAILURE);
              }
            }
          } else {
            conn_sock = events[i].data.fd;
            memset(buffer, 0, sizeof(buffer));
            if ((recv_size = recv(conn_sock, buffer, sizeof(buffer), 0)) == -1 && (errno != EAGAIN)) {
              Soupen_LOG("recv data failed", P(errno));
              exit(EXIT_FAILURE);
            } else if (recv_size == 0) {
              continue;
            }
            memset(response, 0, sizeof(response));
            parser_text(buffer, response);
            if (send(conn_sock, response, strlen(response), 0) == -1 && (errno != EAGAIN) && (errno != EWOULDBLOCK)) {
              perror("send failed");
              exit(EXIT_FAILURE);
            }
          }
        }
      }
      close(sock_);
      close(epfd);
    }
  return ret;
  }
}
