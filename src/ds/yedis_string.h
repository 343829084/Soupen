#ifndef YEDIS_STRING_H_
#define YEDIS_STRING_H_
#include "../base/yedis_common.h"
#include "../base/yedis_memory.h"
namespace yedis_datastructures
{
  class YedisString
  {
  public:
    YedisString();
    YedisString(char *data);
    int append(const YedisString &other);
    int append(const char *p);
    bool is_empty() const {return nullptr == data_;}
    ~YedisString();
    void show() const;
    int length() const {return len_;}
    char *ptr() {return data_;}
    const char *ptr() const {return data_;}
    int compare(char *p);
  private:
    int append_internal(const char *p, size_t len);
  private:
    char *data_;
    size_t capacity_;
    size_t len_;
  };
  class YedisNormalString
  {
  public:
    YedisNormalString(char *data, int len);
    ~YedisNormalString();
    int compare(char *p, int len);
  private:
    char *data_;
    int len_;
  };
}
#endif /*YEDIS_STRING_H_*/
