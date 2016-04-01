#ifndef YEDIS_BLOOM_FILTER_H_
#define YEDIS_BLOOM_FILTER_H_
#include "../ds/yedis_string.h"
namespace yedis_datastructures
{
  class YedisBloomFilter
  {
  public:
    ~YedisBloomFilter();
    void add(const char *key);
    void add(const char *key, int len);
    void add(const YedisString &string);
    bool contains(const char *key) const;
    bool contains(const char *key, int len) const;
    bool contains(const YedisString &string) const;
    int init(int64_t n, int64_t m);
    bool is_inited() {return data_ != nullptr;}
  private:
    void add(uint64_t hash1, uint64_t hash2);
    bool contains(uint64_t hash1, uint64_t hash2) const;
  private:
    uint64_t *data_;
    int64_t size_in_byte_;
    int k;
  public:
    static const int DEFAULT_N = 1024;
    static const int DEFAULT_M = 1000;
  };
}
#endif /*YEDIS_BLOOM_FILTER_H_*/
