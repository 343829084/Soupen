#include "../ds/yedis_bloom_filter.h"
#include "../base/yedis_memory.h"
#include "../base/yedis_hash.h"
#include <cmath>
namespace yedis_datastructures
{
  using namespace yedis_server;
  YedisBloomFilter::~YedisBloomFilter()
  {
    yedis_free(data_, size_in_byte_);
    data_ = nullptr;
    size_in_byte_ = 0;
  }
  int YedisBloomFilter::init(int64_t n, int64_t m)
  {
    int ret = YEDIS_SUCCESS;
    data_ = nullptr;
    k = 0;
    size_in_byte_ = 0;
    if (n <= 0 || m <= 0 || m > n) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      int64_t size = n / 8 * 8 + 8;
      uint64_t *tmp = static_cast<uint64_t *>(yedis_malloc(size));
      if (YEDIS_UNLIKELY(nullptr == tmp)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        MEMSET(tmp, 0, size);
        data_ = tmp;
        size_in_byte_ = size;
        k = log(2.0) * (size_in_byte_ * 8  / m);
        if (k <= 0) {
          k = 2;
        }
      }
    }
    return ret;
  }
  void YedisBloomFilter::add(const char *key)
  {
    uint64_t hash1 = MurmurHash64B(key, strlen(key), SEED1);
    uint64_t hash2 = MurmurHash64B(key, strlen(key), SEED2);
    add(hash1, hash2);
  }
  void YedisBloomFilter::add(const char *key, int len)
  {
    uint64_t hash1 = MurmurHash64B(key, len, SEED1);
    uint64_t hash2 = MurmurHash64B(key, len, SEED2);
    add(hash1, hash2);
  }
  bool YedisBloomFilter::contains(const char *key) const
  {
    uint64_t hash1 = MurmurHash64B(key, strlen(key), SEED1);
    uint64_t hash2 = MurmurHash64B(key, strlen(key), SEED2);
    return contains(hash1, hash2);
  }
  bool YedisBloomFilter::contains(const char *key, int len) const
  {
    uint64_t hash1 = MurmurHash64B(key, len, SEED1);
    uint64_t hash2 = MurmurHash64B(key, len, SEED2);
    return contains(hash1, hash2);
  }
  void YedisBloomFilter::add(const YedisString &key)
  {
    uint64_t hash1 = MurmurHash64B(key.ptr(), key.length(), SEED1);
    uint64_t hash2 = MurmurHash64B(key.ptr(), key.length(), SEED2);
    add(hash1, hash2);
  }
  bool YedisBloomFilter::contains(const YedisString &key) const
  {
    uint64_t hash1 = MurmurHash64B(key.ptr(), key.length(), SEED1);
    uint64_t hash2 = MurmurHash64B(key.ptr(), key.length(), SEED2);
    return contains(hash1, hash2);
  }
  //private functions
  void YedisBloomFilter::add(uint64_t hash1, uint64_t hash2)
  {
    for (int i = 0; i < k; ++i) {
      //hash = hash1 + i * hash2 may be overflowed
      //so
      //(hash1 + i *hash2) % p = (hash1 % p + (i*hash2) % p ) % p
      //i*hash2 % p = i % p * hash2 % p % p;
      //uint64_t idx = hash1 & (size_in_byte_ * 8 - 1) + i & (size_in_byte_ * 8 - 1);
      uint64_t hash = hash1 + i * hash2;
      uint64_t idx = hash & (size_in_byte_ * 8 - 1);
      uint64_t value = data_[idx / 64];
      uint64_t mask = 1 << (63 - idx % 64);
      value |= mask;
      data_[idx / 64] = value;
    }
  }
  bool YedisBloomFilter::contains(uint64_t hash1, uint64_t hash2) const
  {
    bool ret = true;
    for (int i = 0; i < k; ++i) {
      uint64_t hash = hash1 + i * hash2;
      uint64_t idx = hash & (size_in_byte_ * 8 - 1);
      uint64_t value = data_[idx / 64];
      uint64_t mask = 1 << (63 - idx % 64);
      if (!(value & mask)) {
        ret = false;
        break;
      }
    }
    return ret;
  }
}
