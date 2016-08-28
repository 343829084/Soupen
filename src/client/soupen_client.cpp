#include "../client/soupen_client.h"
#include "../base/soupen_memory.h"
#include "../lib/soupen_time.h"
using namespace soupen_lib;
using namespace soupen_datastructures;
namespace soupen_client
{
  int SoupenClient::init(int fd)
  {
    int ret = SOUPEN_SUCCESS;
    fd_ = fd;
    next_ = nullptr;
    last_active_time_ = SoupenTime::get_curr_datetime();
    client_type_ = 0;
    output_buffer_ = input_buffer_ = nullptr;
    has_written_ = 0;
    ret = reset_buffer();
    return ret;
  }
  int SoupenClient::reset_buffer()
  {
    int ret = SOUPEN_SUCCESS;
    if (output_buffer_ != nullptr ) {
      int64_t objectsize = output_buffer_->get_object_size();
      output_buffer_->~SoupenString();
      soupen_free(output_buffer_, objectsize);
      output_buffer_ = nullptr;
      has_written_  = 0;
    }
    if (SOUPEN_FAIL(SoupenString::factory(OUTPUT_BUFFER_SIZE_BYTES, output_buffer_))) {
      Soupen_LOG("reset output buffer failed", P(ret));
    } else if (SOUPEN_FAIL(output_buffer_->SoupenString::init("", OUTPUT_BUFFER_SIZE_BYTES))) {
      Soupen_LOG("reset output buffer failed", P(ret));
    } else {
      output_buffer_->set_length(0);
    }
    if (SOUPEN_SUCCESS == ret && input_buffer_ != nullptr) {
      int64_t objectsize = input_buffer_->get_object_size();
      input_buffer_->~SoupenString();
      input_buffer_ = nullptr;
      soupen_free(input_buffer_, objectsize);
    }
    if (SOUPEN_SUCCESS == ret && SOUPEN_FAIL(SoupenString::factory(INTPUT_BUFFER_SIZE_BYTES, input_buffer_))) {
      Soupen_LOG("reset input buffer failed", P(ret));
    } else if (SOUPEN_FAIL(input_buffer_->SoupenString::init("", INTPUT_BUFFER_SIZE_BYTES))) {
      Soupen_LOG("reset input buffer failed", P(ret));
    } else {
      input_buffer_->set_length(0);
    }
    return ret;
  }
  int SoupenClientManager::create_client(int fd)
  {
    int ret = SOUPEN_SUCCESS;
    if (exist_client(fd)) {
      //already exist. do nothing
    } else {
      SoupenClient *client = static_cast<SoupenClient*>(soupen_malloc(sizeof(SoupenClient)));
      if (SOUPEN_UNLIKELY(client == nullptr)) {
        ret = SOUPEN_ERROR_NO_MEMORY;
        Soupen_LOG("allocate client failed", P(ret));
      } else if (SOUPEN_FAIL(client->init(fd))) {
        Soupen_LOG("init client failed", P(ret));
        soupen_reclaim(client);
      } else if (SOUPEN_UNLIKELY(clients_ == nullptr)) {
        clients_ = client;
      } else {
        client->next_ = clients_;
        clients_ = client;
      }
    }
    return ret;
  }

  bool SoupenClientManager::exist_client(int fd)
  {
    return get_client(fd) != nullptr;
  }

  SoupenClient* SoupenClientManager::get_client(int fd)
  {
    SoupenClient *tmp = clients_;
    while(tmp) {
      if (tmp->fd_ == fd) {
        break;
      }
      tmp = tmp->next_;
    }
    return tmp;
  }

}
