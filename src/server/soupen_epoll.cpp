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
using namespace soupen_client;
using namespace soupen_datastructures;
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
  int SoupenEpoll::process_new_connections(int epfd)
  {
    int ret = SOUPEN_SUCCESS;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char client_ip_str[INET_ADDRSTRLEN];
    while (1) {
      int conn_sock = accept(sock_, (struct sockaddr *) &client_addr, &client_addr_len);
      if (conn_sock == -1) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
          break;
        } else {
          ret = SOUPEN_ERROR_UNEXPECTED;
          Soupen_LOG("accept failed", P(ret), P(errno));
          break;
        }
      }
      if (!inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, sizeof(client_ip_str))) {
        Soupen_LOG("inet_ntop failed", P(errno));
        continue;
      } else {
        printf("accept a client from: %s\n", client_ip_str);
        if (SOUPEN_FAIL(create_client(epfd, conn_sock))) {
          Soupen_LOG("add client failed", P(ret), P(errno), P(client_ip_str));
        }
      }
    }
    return ret;
  }
  int SoupenEpoll::create_client(int epfd, int fd)
  {
    int ret = SOUPEN_SUCCESS;
    struct epoll_event event;
    set_socket_nonblocking(fd);
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.fd = fd;
    if (SOUPEN_FAIL(client_manager.create_client(fd))) {
      Soupen_LOG("add client failed", P(ret), P(errno));
    } else if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1) {
      ret = SOUPEN_ERROR_UNEXPECTED;
      Soupen_LOG("epoll_ctl failed", P(errno));
    }
    return ret;
  }
  int SoupenEpoll::close_client(int epfd, int fd)
  {
    int ret = SOUPEN_SUCCESS;
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) == -1) {
      Soupen_LOG("epoll_ctl failed", P(errno));
    }
    close(fd);
    /*if (SOUPEN_FAIL(client_manager.remove_client(fd))) {
      Soupen_LOG("remove client failed", P(ret));
    }*/
    return ret;
  }
  int SoupenEpoll::read_client(int epfd, int fd)
  {
    int ret = SOUPEN_SUCCESS;
    SoupenClient *client = nullptr;
    char *input_buffer = nullptr;
    if (SOUPEN_UNLIKELY((client = client_manager.get_client(fd))== nullptr)) {
      ret = SOUPEN_ERROR_UNEXPECTED;
      Soupen_LOG("unexpected error. client pointer is null", P(ret), P(fd));
    } else if (SOUPEN_UNLIKELY(client->input_buffer_== nullptr)) {
      ret = SOUPEN_ERROR_UNEXPECTED;
      Soupen_LOG("unexpected error. client input pointer is null", P(ret), P(fd));
    } else if (SOUPEN_UNLIKELY((input_buffer = client->input_buffer_->get_ptr()) == nullptr )) {
      ret = SOUPEN_ERROR_UNEXPECTED;
      Soupen_LOG("unexpected error. client input pointer is null", P(ret), P(fd));
    } else {
      char tmp[1024];
      int bytes_read = 0;
      int bytes_to_read = client->input_buffer_->length();
      while ((bytes_read = recv(fd, tmp, 1024, 0)) > 0) {
        client->input_buffer_->append(tmp, bytes_read);
      }
      if (bytes_read == 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          close_client(epfd, fd);
        } else {
          ret = SOUPEN_ERROR_UNEXPECTED;
          Soupen_LOG("unexpected error. read failed", P(bytes_read), P(fd), P(errno));
        }
      } else if (bytes_read == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          if (SOUPEN_FAIL(parse_cmd(client->input_buffer_->get_ptr(), client))) {
            Soupen_LOG("parse cmd failed", P(ret), P(fd), P(errno));
          } else if (SOUPEN_FAIL(write_client(epfd, fd))) {
            Soupen_LOG("write client failed", P(ret), P(fd), P(errno));
          }
        } else {
          ret = SOUPEN_ERROR_UNEXPECTED;
          Soupen_LOG("unexpected error. read failed", P(bytes_read), P(fd), P(errno));
        }
      } else {
        ret = SOUPEN_ERROR_UNEXPECTED;
        Soupen_LOG("unexpected error. read failed", P(bytes_read), P(fd), P(errno));
      }
    }
    return 0;
  }
  int SoupenEpoll::write_client(int epfd, int fd)
  {
    int ret = SOUPEN_SUCCESS;
    SoupenClient *client = nullptr;
    char *output_buffer = nullptr;
    if (SOUPEN_UNLIKELY((client = client_manager.get_client(fd))== nullptr)) {
      ret = SOUPEN_ERROR_UNEXPECTED;
      Soupen_LOG("unexpected error. client pointer is null", P(ret), P(fd));
    } else if (SOUPEN_UNLIKELY(client->output_buffer_== nullptr)) {
      ret = SOUPEN_ERROR_UNEXPECTED;
      Soupen_LOG("unexpected error. client output pointer is null", P(ret), P(fd));
    } else if (SOUPEN_UNLIKELY((output_buffer = client->output_buffer_->get_ptr()) == nullptr)) {
      ret = SOUPEN_ERROR_UNEXPECTED;
      Soupen_LOG("unexpected error. client output pointer is null", P(ret), P(fd));
    } else {
      int64_t &has_written = client->has_written_;
      int bytes_sent = 0;
      int bytes_to_send = client->output_buffer_->length() - has_written;
      while ((bytes_sent = send(fd, output_buffer + has_written, bytes_to_send, 0)) > 0) {
        has_written += bytes_sent;
        bytes_to_send -= bytes_sent;
      }
      if (bytes_to_send == 0) {
        //all done. nothing need to send.
        if (SOUPEN_FAIL(client->reset_buffer())) {
          Soupen_LOG("reset buffer failed", P(ret));
        }
      } else if (bytes_sent == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          ret = SOUPEN_ERROR_UNEXPECTED;
          Soupen_LOG("epoll_ctl failed", P(errno));
        }
      } else {
        ret = SOUPEN_ERROR_UNEXPECTED;
        Soupen_LOG("unexpected error. send failed", P(ret), P(fd), P(bytes_sent), P(errno));
      }
    }
    return ret;
  }
  int SoupenEpoll::work()
  {
    int ret = SOUPEN_SUCCESS;
    int epfd = -1;
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
      char buffer[BUFF_SIZE];
      int recv_size;
      while (1) {
        int res = epoll_wait(epfd, events, EPOLL_MAXEVENTS, EPOLL_TIMEOUT);
        if (res < 0) {
          Soupen_LOG("epoll_wait error", P(res), P(errno));
          continue;
        } else if (res == 0) {
          fprintf(stderr, "memory used right now: %lld Bytes\n", SoupenServerInfoManager::get_total_memory_used());
          continue;
        }
        //res > 0
        for (int i = 0; i < res; i++) {
          if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
            close_client(epfd, events[i].data.fd);
            continue;
          }
          int connection_fd = events[i].data.fd;
          if (connection_fd == sock_) { //new connection
            process_new_connections(epfd);
          } else if (events[i].events & EPOLLIN) { //read
            read_client(epfd, connection_fd);
          } else if (events[i].events & EPOLLOUT) { //write
            write_client(epfd, connection_fd);
          }
        }
      }
      close(sock_);
      close(epfd);
    }
    return ret;
  }

  //================================================================================================//
  soupen_client::SoupenClientManager SoupenEpoll::client_manager;
}
