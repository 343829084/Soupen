#include "../base/yedis_common.h"
#include "../ds/yedis_string.h"
#include "../server/yedis_global_info.h"
#include<iostream>
using namespace std;
namespace yedis_datastructures
{
  using namespace yedis_server;
  YedisString::YedisString()
  {
    data_ = nullptr;
    len_ = 0;
    capacity_ = 0;
  }
  YedisString::~YedisString()
  {
    yedis_free(data_, capacity_);
    len_ = 0;
    capacity_ = 0;
  }
  YedisString::YedisString(char *data)
  {
    if (nullptr != data) {
      size_t len = strlen(data);
      size_t size = len * sizeof(char);
      size_t capacity = 2 * len;
      char *tmp = static_cast<char*>(yedis_malloc(capacity));
      MEMCPY(tmp, data, size);
      data_ = tmp;
      len_ = len;
      capacity_ = capacity;
    }
  }
  int YedisString::append(const YedisString &other)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(other.len_ < 0 || other.len_ > YEDIS_INT32_MAX - len_)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else if (YEDIS_UNLIKELY(other.is_empty())) {
      //empty string. do nothing
    } else {
      ret = append_internal(other.data_, other.len_);
    }
    return ret;
  }
  int YedisString::append(const char *p)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(nullptr == p)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      size_t len = strlen(p);
      ret = append_internal(p, len);
    }
    return ret;
  }
  int YedisString::append_internal(const char *p, size_t len)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(len > YEDIS_INT32_MAX - len_)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else if (len_ + len <= capacity_) {
      MEMCPY(data_ + len_, p, len * sizeof(char));
      len_ += len;
    } else {
      int new_buffer_len = len_ + len;
      size_t new_buffer_size = 2 * new_buffer_len * sizeof(char);
      char *new_buffer = (char*) yedis_malloc(new_buffer_size);
      if (YEDIS_UNLIKELY(nullptr == new_buffer)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        MEMCPY(new_buffer, data_, len_ * sizeof(char));
        MEMCPY(new_buffer + len_, p, len  * sizeof(char));
        yedis_free(data_, capacity_);
        data_ = new_buffer;
        len_ += len;
        capacity_ = new_buffer_size;
      }
    }
    return ret;
  }
  void YedisString::show() const
  {
    for (size_t i = 0; i < len_; i++)
      cout<<data_[i];
    cout<<endl;
  }
  int YedisString::compare(char *p)
  {
    int ret = 0;
    if (nullptr != p) {
      ret = MEMCMP(data_, p, len_);
    }
    return ret;
  }
  YedisNormalString::YedisNormalString(char *data, int len)
  {
    if (nullptr != data) {
      len_ = len;
      size_t size = len * sizeof(char);
      char *tmp = static_cast<char*>(yedis_malloc(size));
      MEMCPY(tmp, data, size);
      data_ = tmp;
    }
  }
  YedisNormalString::~YedisNormalString()
  {
    yedis_free(data_, len_ * sizeof(char));
  }
  int YedisNormalString::compare(char *p, int len)
  {
    int ret = 0;
    if (nullptr != p) {
      size_t size = len * sizeof(char);
      ret = MEMCMP(data_, p, size);
    }
    return ret;
  }
}
