#ifndef YEDIS_BLOOM_FILTER_H_
#define YEDIS_BLOOM_FILTER_H_
#include "../base/yedis_common.h"
#include "../base/yedis_hash.h"
#include "../ds/yedis_string.h"
#include <cmath>
namespace yedis_datastructures
{
  class YedisBloomFilter
  {
  public:
    YedisBloomFilter(int n, int m);
    ~YedisBloomFilter();
    void add(const char *key);
    void add(const char *key, int len);
    void add(const YedisString &string);
    bool contains(const char *key) const;
    bool contains(const char *key, int len) const;
    bool contains(const YedisString &string) const;
  private:
    int init(int n, int m);
    void add(uint64_t hash1, uint64_t hash2);
    bool contains(uint64_t hash1, uint64_t hash2) const;
  private:
    uint64_t *data_;
    uint64_t size_in_byte_;
    int k;
  public:
    static const int DEFAULT_N = 1024;
    static const int DEFAULT_M = 1000;
  };
}
#endif /*YEDIS_BLOOM_FILTER_H_*/
