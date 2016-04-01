#include "../ds/yedis_string.h"
namespace yedis_datastructures
{
  using namespace yedis_server;
  int YedisString::init()
  {
    data_ = nullptr;
    len_ = 0;
    capacity_ = 0;
    return YEDIS_SUCCESS;
  }
  YedisString::~YedisString()
  {
    if (is_inited()) {
      yedis_free(data_, capacity_);
    }
    len_ = -1;
    capacity_ = -1;
  }
  int YedisString::init(char *data)
  {
    int ret = YEDIS_SUCCESS;
    data_ = nullptr;
    len_ = -1;
    capacity_ = -1;
    if (YEDIS_UNLIKELY(nullptr == data)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      int64_t len = strlen(data);
      int64_t size = len * sizeof(char);
      int64_t capacity = 2 * len;
      char *tmp = static_cast<char*>(yedis_malloc(capacity));
      if (YEDIS_UNLIKELY(nullptr == tmp)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        MEMCPY(tmp, data, size);
        data_ = tmp;
        len_ = len;
        capacity_ = capacity;
      }
    }
    return ret;
  }
  int YedisString::append(const YedisString &other)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(other.len_ < 0 || other.len_ > YEDIS_INT64_MAX - len_)) {
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
      int64_t len = strlen(p);
      ret = append_internal(p, len);
    }
    return ret;
  }
  int YedisString::append_internal(const char *p, int64_t len)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(len <= 0 || len > YEDIS_INT64_MAX - len_)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else if (len_ + len <= capacity_) {
      MEMCPY(data_ + len_, p, len * sizeof(char));
      len_ += len;
    } else {
      int new_buffer_len = len_ + len;
      int64_t new_buffer_size = 2 * new_buffer_len * sizeof(char);
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
  int YedisString::compare(char *p)
  {
    int ret = -1;
    if (nullptr != p) {
      ret = MEMCMP(data_, p, len_);
    }
    return ret;
  }
  int YedisNormalString::init(char *data, int64_t len)
  {
    int ret = YEDIS_SUCCESS;
    data_ = nullptr;
    len_ = -1;
    if (YEDIS_UNLIKELY(nullptr == data || len <= 0)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      int64_t size = len * sizeof(char);
      char *tmp = static_cast<char*>(yedis_malloc(size));
      if (YEDIS_UNLIKELY(nullptr == tmp)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        MEMCPY(tmp, data, size);
        data_ = tmp;
        len_ = len;
      }
    }
    return ret;
  }
  YedisNormalString::~YedisNormalString()
  {
    if (is_inited()) {
      yedis_free(data_, len_ * sizeof(char));
    }
    len_ = -1;
    data_ = nullptr;
  }
  bool YedisNormalString::is_equal(char *p, int64_t len)
  {
    bool ret = (len == len_) && (nullptr != p);
    if (ret) {
      int64_t size = len * sizeof(char);
      ret = (MEMCMP(data_, p, size) == 0);
    }
    return ret;
  }
}
