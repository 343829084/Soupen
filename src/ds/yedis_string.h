#ifndef YEDIS_STRING_H_
#define YEDIS_STRING_H_
#include "../base/yedis_memory.h"
#include <string.h>
namespace yedis_datastructures
{
  class YedisString
  {
  public:
    int init();
    int init(char *data);
    int append(const YedisString &other);
    int append(const char *p);
    bool is_empty() const {return nullptr == non_buffer_data_;}
    ~YedisString();
    int64_t length() const {return len_;}
    char *ptr() {return non_buffer_data_;}
    const char *ptr() const {return non_buffer_data_;}
    int compare(char *p);
    bool is_inited() {return len_ > 0;}
  private:
    int append_internal(const char *p, int64_t len);
  private:
    char *non_buffer_data_;
    int64_t capacity_;
    int64_t len_;
  };
  class YedisNormalString
  {
  public:
    //when len > 0 and len < 64, we will store string in buffer_data_
    int init(char *str, int64_t len);
    int init(char *str);
    ~YedisNormalString();
    YEDIS_MUST_INLINE const char *get_ptr() const
    {
      return data_;
    }
    YEDIS_MUST_INLINE const bool is_equal(const char *p) const
    {
      return !strcasecmp(p, data_);
    }
    YEDIS_MUST_INLINE const bool is_equal(const char *p, int64_t len) const
    {
      return !strncasecmp(p, data_, len < len_ ? len : len_);
    }
    YEDIS_MUST_INLINE const bool is_equal(const YedisNormalString *other) const
    {
      return 0 == MEMCMP((void *)other->data_, (void *)data_, this->length());
    }
    YEDIS_MUST_INLINE const int cmp(const char *p) const
    {
      return strcasecmp(p, data_);
    }
    YEDIS_MUST_INLINE const int cmp(const YedisNormalString *other) const
    {
      return strcasecmp(other->data_, data_);
    }
    YEDIS_MUST_INLINE const int64_t get_object_size() const
    {
      return len_ < CHAR_LEN_THRESHOLD ? sizeof(YedisNormalString) + len_ + 1 : sizeof(YedisNormalString);
    }
    YEDIS_MUST_INLINE const int64_t length() const
    {
      return len_;
    }
    YEDIS_MUST_INLINE const bool is_inited() const
    {
      return len_ > 0;
    }
    static int factory(const char *p, YedisNormalString* &yn_str);
  private:
    static const int64_t CHAR_LEN_THRESHOLD = 40;
    int64_t len_;
    char *data_;
    char buffer_data_[0];
  };
}
#endif /*YEDIS_STRING_H_*/
