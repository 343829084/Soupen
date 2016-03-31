#ifndef YEDIS_KDTREE_H_
#define YEDIS_KDTREE_H_
#include "../base/yedis_common.h"
namespace yedis_datastructures
{
  template<typename T, int64_t dim>
  struct YedisKDTreeNode
  {
  public:
    YedisKDTreeNode() { left_ = nullptr; right_ = nullptr;}
    //dangerous
    void set(int index, const T &obj) { data_[index] = obj;}
  public:
    YedisKDTreeNode *left_;
    YedisKDTreeNode *right_;
    T data_[dim];
  };
  template<typename T, int64_t dim, typename CallBack>
  class YedisKDtree
  {
  public:
    YEDIS_MUST_INLINE YedisKDtree() { root_ = nullptr; }
    ~YedisKDtree();
    int insert(T *p);
    void query_range(T &lower_bound, T &upper_bound);
  private:
    void gc_help(T *p, CallBack &cb);
    int insert_help(T **trans, T *p, int level);
    void query_range_help(T &lower_bound, T &upper_bound, T *p, int level);
    bool is_match(T &lower_bound, T &upper_bound, T *p);
  private:
    T *root_;
  };

  template<typename T, int64_t dim, typename CallBack>
  YedisKDtree<T, dim, CallBack>::~YedisKDtree()
  {
    CallBack cb;
    gc_help(root_, cb);
  }

  template<typename T, int64_t dim, typename CallBack>
  void YedisKDtree<T, dim, CallBack>:: gc_help(T *p, CallBack &cb)
  {
    if (p != nullptr) {
      gc_help(p->left_, cb);
      gc_help(p->right_, cb);
      cb(p);
    }
  }

  template<typename T, int64_t dim, typename CallBack>
  int YedisKDtree<T, dim, CallBack>::insert_help(T **root, T *p, int level)
  {
    int ret = YEDIS_SUCCESS;
    T **trans = root;
    while (nullptr != *trans) {
      if((*trans)->data_[level] > p->data_[level]) {
        trans = &((*trans)->left_);
      } else {
        trans = &((*trans)->right_);
      }
      level = ++level % dim;
    }
    *trans = p;
    return ret;
  }

  template<typename T, int64_t dim, typename CallBack>
  void YedisKDtree<T, dim, CallBack>::query_range_help(T &lower_bound, T &upper_bound, T *p, int level)
  {
    if (nullptr != p) {
      if (is_match(lower_bound, upper_bound, p)) {
        //find !
      }
      if (lower_bound.data_[level] <= p->data_[level]) {
        query_range_help(lower_bound, upper_bound, p->left_, ++level % dim);
      }
      if (upper_bound.data_[level] >= p->data_[level]) {
        query_range_help(lower_bound, upper_bound, p->right_, ++level % dim);
      }
    }
  }

  template<typename T, int64_t dim, typename CallBack>
  bool YedisKDtree<T, dim, CallBack>::is_match(T &lower_bound, T &upper_bound, T *p)
  {
    for(int64_t i = 0; i < dim; ++i) {
      if (lower_bound.data_[i] > p->data_[i]
          || upper_bound.data_[i] < p->data_[i])
        return false;
    }
    return true;
  }

  template<typename T, int64_t dim, typename CallBack>
  void YedisKDtree<T, dim, CallBack>::query_range(T &lower_bound, T &upper_bound)
  {
    query_range_help(lower_bound, upper_bound, root_, 0);
  }

  template<typename T, int64_t dim, typename CallBack>
  int YedisKDtree<T, dim, CallBack>::insert(T *p)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(nullptr == p)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      ret = insert_help(&root_, p, 0);
    }
    return ret;
  }
}
#endif //YEDIS_KDTREE_H_
