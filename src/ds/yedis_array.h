#ifndef YEDIS_ARRAY_H_
#define YEDIS_ARRAY_H_
#include "../base/yedis_common.h"
#include <new>
namespace yedis_datastructures
{
  template<typename T, int64_t TABLE_SIZE>
  class YedisArray
  {
  public:
    YedisArray();
    virtual ~YedisArray();
    int push_back(const T &obj);
    int pop();
    int top(T &obj) const;
    int at(int index, T &obj);
    T& at(int index);
    const T& at(int index) const;
    YEDIS_MUST_INLINE int64_t get_capacity() const {return table_counts_ * TABLE_SIZE;}
    YEDIS_MUST_INLINE int64_t get_size() const {return count_;}
  private:
    int init();
    int re_alloc();
    void get_table_slot_idx(int64_t index, int64_t &tbl_idx, int64_t &slot_idx) const;
  private:
    T **data_;
    int table_counts_;
    //the total number of objs
    int count_;
    //track the number of objs which have been constructed already.
    //note that , these objs will be reused when possible.
    //they will be reclaimed only when the destructor of YedisArray called
    //we will never decrease the value of obj_allocated_count_.
    int obj_allocated_count_;
    bool is_inited_;
  };
  template<typename T, int64_t TABLE_SIZE>
  YedisArray<T, TABLE_SIZE>::YedisArray()
  {
    init();
  }
  template<typename T, int64_t TABLE_SIZE>
  int YedisArray<T, TABLE_SIZE>::init()
  {
    int ret = YEDIS_SUCCESS;
    char *p = nullptr;
    T **tmp = nullptr;
    if (nullptr == (tmp = new (std::nothrow) T*[1])) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (nullptr == (p = new (std::nothrow) char[sizeof(T) * TABLE_SIZE])) {
      ret = YEDIS_ERROR_NOT_INITED;
      delete []tmp;
    } else {
      tmp[0] = reinterpret_cast<T*>(p);
      data_ = tmp;
      count_ = 0;
      obj_allocated_count_ = 0;
      table_counts_ = 1;
      is_inited_ = true;
    }
    return ret;
  }
  template<typename T, int64_t TABLE_SIZE>
  YedisArray<T, TABLE_SIZE>::~YedisArray()
  {
    int num = 0;
    bool flag = true;
    for(int i = 0; flag && i < table_counts_; ++i) {
      for (int j = 0; j < TABLE_SIZE; j++) {
        if (num < obj_allocated_count_) {
          data_[i][j].~T();
          ++num;
        } else {
          flag = false;
          break;
        }
      }
    }
    if (nullptr != data_) {
      delete []data_;
    }
    obj_allocated_count_ = 0;
    count_ = 0;
    data_ = nullptr;
    table_counts_ = 0;
  }
  template<typename T, int64_t TABLE_SIZE>
  int YedisArray<T, TABLE_SIZE>::push_back(const T &obj)
  {
    int ret = YEDIS_SUCCESS;
    int64_t tbl_idx = 0;
    int64_t slot_idx = 0;
    get_table_slot_idx(count_, tbl_idx, slot_idx);
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (count_ < get_capacity()) {
      if (count_ < obj_allocated_count_) {
        data_[tbl_idx][slot_idx] = obj;//just reuse old objs
      } else {
        new (&data_[tbl_idx][slot_idx]) T(obj); //construct a new obj
        ++obj_allocated_count_;
      }
      ++count_;
    } else {
      if (YEDIS_SUCCESS == (ret = re_alloc())) {
        new (&data_[tbl_idx][0]) T(obj);
        ++obj_allocated_count_;
        ++count_;
      }
    }
    return ret;
  }
  template<typename T, int64_t TABLE_SIZE>
  int YedisArray<T, TABLE_SIZE>::pop()
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (count_ <= 0) {
      ret = YEDIS_ERROR_EMPTY;
    } else {
      count_--;
    }
    return ret;
  }
  template<typename T, int64_t TABLE_SIZE>
  int YedisArray<T, TABLE_SIZE>::top(T &obj) const
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(count_ <= 0)) {
      ret = YEDIS_ERROR_EMPTY;
    } else {
      int64_t tbl_idx = 0;
      int64_t slot_idx = 0;
      get_table_slot_idx(count_ - 1, tbl_idx, slot_idx);
      obj = data_[tbl_idx][slot_idx];
    }
    return ret;
  }
  template<typename T, int64_t TABLE_SIZE>
  int YedisArray<T, TABLE_SIZE>::re_alloc()
  {
    int ret = YEDIS_SUCCESS;
    T **tmp = nullptr;
    char *p = nullptr;
    if (nullptr == (tmp = new T*[table_counts_ + 1])) {
      ret = YEDIS_ERROR_NO_MEMORY;
    } else if (nullptr == (p = new char[sizeof(T) * TABLE_SIZE])) {
      ret = YEDIS_ERROR_NO_MEMORY;
      delete [] tmp;
    } else {
      for (int i = 0 ; i < table_counts_; ++i) {
        tmp[i] = data_[i];
      }
      tmp[table_counts_] = reinterpret_cast<T*>(p);
      delete []data_;
      data_ = tmp;
      ++table_counts_;
    }
    return ret;
  }
  template<typename T, int64_t TABLE_SIZE>
  int YedisArray<T, TABLE_SIZE>::at(int index, T &obj)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(index < 0 || index >= count_)) {
      ret = YEDIS_ERROR_INDEX_OUT_OF_RANGE;
    } else {
      int64_t tbl_idx = 0;
      int64_t slot_idx = 0;
      get_table_slot_idx(index, tbl_idx, slot_idx);
      obj = data_[tbl_idx][slot_idx];
    }
    return ret;
  }
  template<typename T, int64_t TABLE_SIZE>
  T& YedisArray<T, TABLE_SIZE>::at(int index)
  {
    int64_t tbl_idx = 0;
    int64_t slot_idx = 0;
    get_table_slot_idx(index, tbl_idx, slot_idx);
    return data_[tbl_idx][slot_idx];
  }
  template<typename T, int64_t TABLE_SIZE>
  const T& YedisArray<T, TABLE_SIZE>::at(int index) const
  {
    int64_t tbl_idx = 0;
    int64_t slot_idx = 0;
    get_table_slot_idx(index, tbl_idx, slot_idx);
    return data_[tbl_idx][slot_idx];
  }
  template<typename T, int64_t TABLE_SIZE>
  void YedisArray<T, TABLE_SIZE>::get_table_slot_idx(int64_t index, int64_t &tbl_idx, int64_t &slot_idx) const
  {
    tbl_idx = index / TABLE_SIZE;
    slot_idx = index % TABLE_SIZE;//maybe, yeah, maybe slot_idx = index - tbl_idx * TABLE_SIZE will be faster.
  }
}

#endif /* YEDIS_ARRAY_H_ */
