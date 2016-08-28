#ifndef SOUPEN_STRING_H_
#define SOUPEN_STRING_H_
#include "../base/soupen_memory.h"
namespace soupen_datastructures
{
  class SoupenString
  {
  public:
    int init(const char *str, int64_t len);
    int init(const char *str);
    int append(const char *p, int64_t len);
    int append(const char *p);
    ~SoupenString();
    SOUPEN_MUST_INLINE const char *get_ptr() const
    {
      return data_;
    }
    SOUPEN_MUST_INLINE char *get_ptr()
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
      return len_ == len && 0 == MEMCMP(data_, p, len);
    }
    SOUPEN_MUST_INLINE bool is_equal(const SoupenString *other) const
    {
      return len_ == other->length() && 0 == cmp(other);
    }
    //cmp
    SOUPEN_MUST_INLINE int cmp(const char *p) const
    {
      return strcmp(p, data_);
    }
    SOUPEN_MUST_INLINE int cmp(const SoupenString *other) const
    {
      return cmp(other->get_ptr());
    }


    SOUPEN_MUST_INLINE int64_t get_object_size() const
    {
      return capacity_ <= CHAR_LEN_THRESHOLD ? sizeof(SoupenString) + CHAR_LEN_THRESHOLD  + 1 : sizeof(SoupenString);
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
    static int factory(int64_t capacity, SoupenString* &yn_str);
    void set_length(int64_t len) {len_ = len;}
  private:
    static const int64_t CHAR_LEN_THRESHOLD = 39;
    int64_t len_;
    int64_t capacity_;
    char *data_;
    char buffer_data_[0];
  };
  typedef SoupenString SoupenOutputBuffer;
  typedef SoupenString SoupenInputBuffer;
}
#endif /*SOUPEN_STRING_H_*/
