#ifndef YEDIS_STRING_H_
#define YEDIS_STRING_H_
#include "../base/yedis_memory.h"
namespace yedis_datastructures
{
  class YedisString
  {
  public:
    int init();
    int init(char *data);
    int append(const YedisString &other);
    int append(const char *p);
    bool is_empty() const {return nullptr == data_;}
    ~YedisString();
    int64_t length() const {return len_;}
    char *ptr() {return data_;}
    const char *ptr() const {return data_;}
    int compare(char *p);
    bool is_inited() {return len_ != -1;}
  private:
    int append_internal(const char *p, int64_t len);
  private:
    char *data_;
    int64_t capacity_;
    int64_t len_;
  };
  class YedisNormalString
  {
  public:
    int init(char *data, int64_t len);
    ~YedisNormalString();
    bool is_equal(char *p, int64_t len);
    int64_t length() {return len_;}
    bool is_inited() {return len_ != -1;}
  private:
    char *data_;
    int64_t len_;
  };
}
#endif /*YEDIS_STRING_H_*/
