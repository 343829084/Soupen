#ifndef SOUPEN_CLIENT_H_
#define SOUPEN_CLIENT_H_
#include "../base/soupen_define.h"
#include "../ds/soupen_string.h"
namespace soupen_client
{
  struct SoupenClient
  {
    int init(int fd);
    int reset_buffer();
    soupen_datastructures::SoupenOutputBuffer *output_buffer_;
    soupen_datastructures::SoupenInputBuffer *input_buffer_;
    int fd_;
    int64_t has_written_;
    SoupenClient *next_;
    int64_t last_active_time_;
    int client_type_;
    static const int64_t OUTPUT_BUFFER_SIZE_BYTES = 1 * 1024;
    static const int64_t INTPUT_BUFFER_SIZE_BYTES = 1 * 1024;
  };
  class SoupenClientManager
  {
  public:
    SoupenClientManager()
    {
      clients_ = nullptr;
      client_nums_ = 0;
    }
    int create_client(int fd);
    int remove_client(int fd);
    int is_alive(int fd);
    bool exist_client(int fd);
    int update_active_time(int64_t time);
    int get_client_nums();
    SoupenClient* get_client(int fd);
  private:
    SoupenClient *clients_;
    int client_nums_;
  };
}
#endif /*SOUPEN_CLIENT_H_*/
