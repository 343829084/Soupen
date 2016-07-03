#include "../ds/soupen_string.h"
namespace soupen_datastructures
{
  int SoupenString::init(char *str, int64_t len)
  {
    int ret = SOUPEN_SUCCESS;
    data_ = nullptr;
    len_ = -1;
    if (SOUPEN_UNLIKELY(nullptr == str || len <= 0)) {
      ret = SOUPEN_ERROR_INVALID_ARGUMENT;
    } else if (SOUPEN_LIKELY(len < CHAR_LEN_THRESHOLD)) {
      MEMCPY(buffer_data_, str, len);
      buffer_data_[len] = '\0';
      len_ = len;
      data_ = buffer_data_;
    } else {
      char *tmp = static_cast<char*>(soupen_malloc(len + 1));
      if (SOUPEN_UNLIKELY(nullptr == tmp)) {
        ret = SOUPEN_ERROR_NO_MEMORY;
      } else {
        MEMCPY(tmp, str, len);
        tmp[len] = '\0';
        data_ = tmp;
        len_ = len;
      }
    }
    return ret;
  }
  int SoupenString::init(char *str)
  {
    int ret = SOUPEN_SUCCESS;
    data_ = nullptr;
    len_ = -1;
    int64_t len = 0;
    if (SOUPEN_UNLIKELY(nullptr == str || (len = strlen(str)) <= 0)) {
      ret = SOUPEN_ERROR_INVALID_ARGUMENT;
    } else if (SOUPEN_LIKELY(len < CHAR_LEN_THRESHOLD)) {
      ret = init(str, len);
    }
    return ret;
  }
  SoupenString::~SoupenString()
  {
    if (len_ >= CHAR_LEN_THRESHOLD && data_ != nullptr) {
      soupen_free(data_, len_ + 1);// + 1 for '\0'
    }
    data_ = nullptr;
    len_ = -1;
  }
  int SoupenString::factory(const char *p, SoupenString* &yn_str)
  {
    int ret = SOUPEN_SUCCESS;
    int64_t len = 0;
    yn_str = nullptr;
    SoupenString *tmp = nullptr;
    if (SOUPEN_UNLIKELY(nullptr == p || (len = strlen(p)) <= 0)) {
      ret = SOUPEN_ERROR_INVALID_ARGUMENT;
    } else {
      ret = factory(p, len, yn_str);
    }
    return ret;
  }

  int SoupenString::factory(const char *p, int64_t len, SoupenString* &yn_str)
  {
    int ret = SOUPEN_SUCCESS;
    yn_str = nullptr;
    SoupenString *tmp = nullptr;
    if (SOUPEN_UNLIKELY(nullptr == p)) {
      ret = SOUPEN_ERROR_INVALID_ARGUMENT;
    } else if (SOUPEN_LIKELY(len < CHAR_LEN_THRESHOLD)) {
      tmp = static_cast<SoupenString*>(soupen_malloc(sizeof(SoupenString) + len + 1));
    } else {
      tmp = static_cast<SoupenString*>(soupen_malloc(sizeof(SoupenString)));
    }
    if (SOUPEN_LIKELY(SOUPEN_SUCCESS == ret)) {
      if (SOUPEN_UNLIKELY(nullptr == tmp)) {
        ret = SOUPEN_ERROR_NO_MEMORY;
      } else {
        yn_str = tmp;
      }
    }
    return ret;
  }
}
