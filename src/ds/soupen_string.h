#ifndef SOUPEN_STRING_H_
#define SOUPEN_STRING_H_
#include "../base/soupen_memory.h"
namespace soupen_datastructures
{
  class SoupenString
  {
  public:
    int init(char *str, int64_t len);
    int init(char *str);
    ~SoupenString();
    SOUPEN_MUST_INLINE const char *get_ptr() const
    {
      return data_;
    }
    //is equal
    SOUPEN_MUST_INLINE bool is_equal(const char *p) const
    {
      return 0 == cmp(p);
    }
    SOUPEN_MUST_INLINE bool is_equal(const char *p, int64_t len) const
    {
      return 0 == cmp(p, len);
    }
    SOUPEN_MUST_INLINE bool is_equal(const SoupenString *other) const
    {
      return 0 == cmp(other->get_ptr(), other->length());
    }
    //cmp
    SOUPEN_MUST_INLINE int cmp(const char *p) const
    {
      return strcmp(p, data_);
    }
    SOUPEN_MUST_INLINE int cmp(const char *p, int64_t len) const
    {
      return MEMCMP(data_, p, this->length() < len ? this->length() : len);
    }
    SOUPEN_MUST_INLINE int cmp(const SoupenString *other) const
    {
      return cmp(other->get_ptr(), other->length());
    }
    SOUPEN_MUST_INLINE int64_t get_object_size() const
    {
      return len_ < CHAR_LEN_THRESHOLD ? sizeof(SoupenString) + len_ + 1 : sizeof(SoupenString);
    }
    SOUPEN_MUST_INLINE int64_t length() const
    {
      return len_;
    }
    SOUPEN_MUST_INLINE bool is_empty() const
    {
      return data_ == nullptr || len_ == 0;
    }
    static int factory(const char *p, SoupenString* &yn_str);
    static int factory(const char *p, int64_t len, SoupenString* &yn_str);
  private:
    static const int64_t CHAR_LEN_THRESHOLD = 48;
    int64_t len_;
    char *data_;
    char buffer_data_[0];
  };
}
#endif /*SOUPEN_STRING_H_*/
