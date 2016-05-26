#include "../ds/yedis_string.h"
namespace yedis_datastructures
{
  int YedisString::init(char *str, int64_t len)
  {
    int ret = YEDIS_SUCCESS;
    data_ = nullptr;
    len_ = -1;
    if (YEDIS_UNLIKELY(nullptr == str || len <= 0)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else if (YEDIS_LIKELY(len < CHAR_LEN_THRESHOLD)) {
      MEMCPY(buffer_data_, str, len);
      buffer_data_[len] = '\0';
      len_ = len;
      data_ = buffer_data_;
    } else {
      char *tmp = static_cast<char*>(yedis_malloc(len + 1));
      if (YEDIS_UNLIKELY(nullptr == tmp)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        MEMCPY(tmp, str, len);
        tmp[len] = '\0';
        data_ = tmp;
        len_ = len;
      }
    }
    return ret;
  }
  int YedisString::init(char *str)
  {
    int ret = YEDIS_SUCCESS;
    data_ = nullptr;
    len_ = -1;
    int64_t len = 0;
    if (YEDIS_UNLIKELY(nullptr == str || (len = strlen(str)) <= 0)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else if (YEDIS_LIKELY(len < CHAR_LEN_THRESHOLD)) {
      ret = init(str, len);
    }
    return ret;
  }
  YedisString::~YedisString()
  {
    if (len_ >= CHAR_LEN_THRESHOLD && data_ != nullptr) {
      yedis_free(data_, len_);
    }
    data_ = nullptr;
    len_ = -1;
  }
  int YedisString::factory(const char *p, YedisString* &yn_str)
  {
    int ret = YEDIS_SUCCESS;
    int64_t len = 0;
    yn_str = nullptr;
    YedisString *tmp = nullptr;
    if (YEDIS_UNLIKELY(nullptr == p || (len = strlen(p)) <= 0)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else if (YEDIS_LIKELY(len < CHAR_LEN_THRESHOLD)) {
      tmp = static_cast<YedisString*>(yedis_malloc(sizeof(YedisString) + len + 1));
    } else {
      tmp = static_cast<YedisString*>(yedis_malloc(sizeof(YedisString)));
    }
    if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) {
      if (YEDIS_UNLIKELY(nullptr == tmp)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        yn_str = tmp;
      }
    }
    return ret;
  }
}
