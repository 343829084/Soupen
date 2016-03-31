#ifndef YEDIS_CUCKOO_HASHMAP_H_
#define YEDIS_CUCKOO_HASHMAP_H_
#include "../base/yedis_common.h"
#include <new>
namespace yedis_datastructures
{
  typedef uint64_t (*HashFunc)(const int64_t key, uint64_t mod);
  static uint64_t hash_func1(const int64_t key, uint64_t mod)
  {
    return ((uint64_t)key) % mod;
  }
  static uint64_t hash_func2(const int64_t key, uint64_t mod)
  {
    return ((uint64_t)(3 * key + 1)) % mod;//<TODO> bad hash function.
  }
  template<typename Key, typename Value, uint64_t slot_num, uint16_t bucket_num>
  class YedisCukooHashMap
  {
  private:
    struct Entry
    {
    public:
      Entry() {key_ = INT64_MIN;}
      YEDIS_MUST_INLINE bool is_vacant() {return INT64_MIN == key_;}
      int64_t key_;
      Value value_;
    };
  public:
    YedisCukooHashMap();
    virtual ~YedisCukooHashMap(){destroy(data_, slot_num_);}
    int init();
    int set(const Key &key, const Value &value);
    int get(const Key &key, const Value *&value);
    bool contains(const Key &key);
    YEDIS_MUST_INLINE int64_t get_count() {return count_;}
  private:
    void destroy(Entry **p, int num);
    int find_in_table(uint16_t idx, const Key &key, const Value *&value);
    int try_insert_in_table(uint16_t idx, const Key &key, const Value &value);
    bool is_need_rehashing(const Key &key, const Value &value);
    int replace(const Key &key, const Value &value);
    bool is_vacant(uint16_t idx, const Key &key, uint16_t &pos, uint64_t &slot_idx);
    YEDIS_MUST_INLINE int64_t get_max_loops_size() {return 2 * slot_num_ * bucket_num_ / 3;}
  private:
    Entry **data_;
    uint64_t slot_num_;
    uint16_t bucket_num_;
    int64_t count_;
    bool is_inited_;
    HashFunc hash_funcs_[2];
  };
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  YedisCukooHashMap<Key, Value, slot_num, bucket_num>::YedisCukooHashMap()
  {
    init();
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  int YedisCukooHashMap<Key, Value, slot_num, bucket_num>::init()
  {
    int ret = YEDIS_SUCCESS;
    if (slot_num <= 0 || bucket_num <= 0) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      Entry **tmp = nullptr;
      if (nullptr == (tmp = new (std::nothrow) Entry*[slot_num])) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        for (int64_t i = 0; i < slot_num; ++i) {
          tmp[i] = new (std::nothrow) Entry[bucket_num];
          if (nullptr == tmp[i]) {//if it failed, we should reclaim the memory for first to i-1th tables;
            destroy(tmp, i);
            ret = YEDIS_ERROR_NO_MEMORY;
            break;
          }
        }//end for
        if (YEDIS_SUCCESS == ret) {
          data_ = tmp;
          slot_num_ = slot_num;
          bucket_num_ = bucket_num;
          count_ = 0;
          hash_funcs_[0] = hash_func1;
          hash_funcs_[1] = hash_func2;
          is_inited_ = true;
        }
      }
    }//end else
    return ret;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  void YedisCukooHashMap<Key, Value, slot_num, bucket_num>::destroy(Entry **p, int num)
  {
    for (int i = 0; i < num; ++i) {
      delete []p[i];
      p[i] = nullptr;
    }
    delete []p;
    p = nullptr;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  int YedisCukooHashMap<Key, Value, slot_num, bucket_num>::set(const Key &key, const Value &value)
  {
    int ret = YEDIS_SUCCESS;
    if (!is_inited_) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (key == INT64_MIN) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;//int64_min has been used for vacant flag
    } else if (YEDIS_SUCCESS == (ret = try_insert_in_table(0, key, value))) {
      ++count_;
    } else if (YEDIS_SUCCESS == (ret = try_insert_in_table(1, key, value))) {
      ++count_;
    } else if (YEDIS_ERROR_TABLE_FULL != ret){
      ret = YEDIS_ERROR_UNEXPECTED;
    } else if (is_need_rehashing(key, value)) {
      //need to rehashing....
      ret = YEDIS_ERROR_TABLE_NEED_RESIZE;
    } else if (YEDIS_SUCCESS != (ret = replace(key, value))) {
      ret = YEDIS_ERROR_UNEXPECTED;
    }
    return ret;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  inline int YedisCukooHashMap<Key, Value, slot_num, bucket_num>::get(const Key &key, const Value *&value)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
     //no need to check value is null or not here.
    } else if(YEDIS_LIKELY(YEDIS_SUCCESS == find_in_table(0, key, value))) {
      //do nothing
    } else if(YEDIS_SUCCESS != find_in_table(1, key, value)) {
      ret = YEDIS_ERROR_ENTRY_NOT_EXIST;
    }
    return ret;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  YEDIS_MUST_INLINE bool YedisCukooHashMap<Key, Value, slot_num, bucket_num>::contains(const Key &key)
  {
    const Value *tmp = nullptr;
    return get(key, tmp) == YEDIS_SUCCESS;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  int YedisCukooHashMap<Key, Value, slot_num, bucket_num>::find_in_table(uint16_t idx, const Key &key, const Value *&value)
  {
    int ret = YEDIS_ERROR_ENTRY_NOT_EXIST;
    uint64_t slot_idx = hash_funcs_[idx](key, slot_num_);
    for (uint16_t i = 0; i < bucket_num_; ++i) {
      if (data_[slot_idx][i].key_ == key) {
        ret = YEDIS_SUCCESS;
        value = &(data_[slot_idx][i].value_);
        break;
      }
    }
    return ret;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  int YedisCukooHashMap<Key, Value, slot_num, bucket_num>::try_insert_in_table(uint16_t idx, const Key &key, const Value &value)
  {
    int ret = YEDIS_ERROR_TABLE_FULL;
    uint16_t pos = 0;
    uint64_t slot_idx = 0;
    if(is_vacant(idx, key, pos, slot_idx)) {
      data_[slot_idx][pos].value_ = value;
      data_[slot_idx][pos].key_ = key;
      ret = YEDIS_SUCCESS;
    }
    return ret;
  }
  //after function returned
  //slot_idx and pos indicate an vacant location to store an element
  //will be used by is_need_rehashing
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  bool YedisCukooHashMap<Key, Value, slot_num, bucket_num>::is_vacant(uint16_t idx, const Key &key, uint16_t &pos, uint64_t &slot_idx)
  {
    bool ret = false;
    slot_idx = hash_funcs_[idx](key, slot_num_);
    for (uint16_t i = 0; i < bucket_num_; ++i) {
      if (data_[slot_idx][i].is_vacant()) {
        ret = true;
        pos = i;
      }
    }
    return ret;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  int YedisCukooHashMap<Key, Value, slot_num, bucket_num>::replace(const Key &key, const Value &value)
  {
    int ret = YEDIS_SUCCESS;
    static const uint16_t BUCKET_IDX_TO_EVICTED = 0;
    int64_t try_steps = 0;
    uint64_t slot_idx = 0;
    Key next_key = key;
    Value next_value = value;
    Key tmp_key = key;
    Value tmp_value = value;
    uint16_t pos = 0;
    int64_t max_steps = get_max_loops_size();
    while(try_steps < max_steps) {
      ++try_steps;
      if (is_vacant(0, next_key, pos, slot_idx)) {
        data_[slot_idx][pos].key_ = next_key;
        data_[slot_idx][pos].value_ = next_value;
        break;
      } else if (is_vacant(1, next_key, pos, slot_idx)) {
        data_[slot_idx][pos].key_ = next_key;
        data_[slot_idx][pos].value_ = next_value;
        break;
      } else {
        tmp_key = data_[slot_idx][BUCKET_IDX_TO_EVICTED].key_;
        tmp_value = data_[slot_idx][BUCKET_IDX_TO_EVICTED].value_;
        data_[slot_idx][BUCKET_IDX_TO_EVICTED].key_ = next_key;
        data_[slot_idx][BUCKET_IDX_TO_EVICTED].value_ = next_value;
        next_key = tmp_key;
        next_value = tmp_value;
      }
    }
    if (try_steps >= max_steps) {
      ret = YEDIS_ERROR_TABLE_NEED_RESIZE;
    }
    return ret;
  }
  template<typename Key , typename Value, uint64_t slot_num, uint16_t bucket_num>
  bool YedisCukooHashMap<Key, Value, slot_num, bucket_num>::is_need_rehashing(const Key &key, const Value &value)
  {
    bool ret = false;
    static const uint16_t BUCKET_IDX_TO_EVICTED = 0;
    int64_t try_steps = 0;
    uint64_t slot_idx = 0;
    Key next_key = key;
    Value next_value = value;
    Key tmp_key = key;
    Value tmp_value = value;
    uint16_t pos = 0;
    int64_t max_steps = get_max_loops_size();
    while(try_steps < max_steps) {
      ++try_steps;
      if (is_vacant(0, next_key, pos, slot_idx)) {
        break;
      } else if (is_vacant(1, next_key, pos, slot_idx)) {
        break;
      } else {
        next_key = data_[slot_idx][BUCKET_IDX_TO_EVICTED].key_;;
        next_value = data_[slot_idx][BUCKET_IDX_TO_EVICTED].value_;
      }
    }
    if (try_steps >= max_steps) {
      ret = true;
    }
    return ret;
  }
}
#endif /*YEDIS_CUCKOO_HASHMAP_H_*/
