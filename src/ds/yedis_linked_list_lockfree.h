#ifndef YEDIS_LINKED_LIST_LOCKFREE_H_
#define YEDIS_LINKED_LIST_LOCKFREE_H_
#include "../base/yedis_common.h"
namespace yedis_datastructures
{
  struct LinkedListNode
  {
    LinkedListNode(int64_t value, LinkedListNode *next) :
      value_(value), next_(next) {}
    LinkedListNode(LinkedListNode *next) : next_(next) {}
    int compare(const LinkedListNode *p)
    {
      int ret = 0;
      if (value_ > p->value_) {
        ret = 1;
      } else if (value_ < p->value_) {
        ret = -1;
      } //no else here
      return ret;
    }
    int64_t value_;
    LinkedListNode *next_;
  };
  template<typename T>
  class YedisLinkedListLF
  {
  public:
    YedisLinkedListLF()
    {
      init();
    }
    int init()
    {
      int ret = YEDIS_SUCCESS;
      if (nullptr == (tail_ = new T(nullptr))) {
        abort();
      } else if (nullptr == (head_ = new T(tail_))) {
        if (nullptr != tail_) {
          delete tail_;
          abort();
        }
      }
      return ret;
    }
    int insert(T* &p);
    int del(const T *p);
    bool contains(const T *value);
    void show()
    {
      for (const T* tmp = head_->next_; tmp != tail_; tmp = tmp->next_) {

      }
    }
  private:
    /*it holds that :
      1. left is unmarked
      2. left->value_ <=  node->value_
    */

    int find(T *p, T* &left, T* &right);
    inline static bool is_marked(const T * const node)
    {
      return reinterpret_cast<uint64_t>(node) & 1;
    }
    inline static bool is_unmarked(const T * const node)
    {
      return !is_marked(node);
    }
    inline static void mark(const T* &node)
    {
      uint64_t tmp = reinterpret_cast<uint64_t>(node);
      node = reinterpret_cast<T*>(tmp | 1);
    }
    inline static void unmark(const T* &node)
    {
      uint64_t tmp = reinterpret_cast<uint64_t>(node);
      node = reinterpret_cast<T*>((tmp) & (~1ULL));
    }
    inline static T* get_unmark_pointer(T* const node)
    {
      uint64_t tmp = reinterpret_cast<uint64_t>(node);
      return reinterpret_cast<T*>((tmp) & (~1ULL));
    }
  private:
    T *head_;
    T *tail_;
  };
  template<typename T>
  int YedisLinkedListLF<T>::insert(T* &p)
  {
    int ret = YEDIS_SUCCESS;
    T *left = nullptr;
    T *right = nullptr;
    while(true) {
      find(p, left, right);
      p->next_ = right;
      if (CAS(&(left->next_), right, p))
        break;
    }
    return ret;
  }
  template<typename T> int YedisLinkedListLF<T>::find(T *p, T* &left, T* &right)
  {
    int ret = YEDIS_SUCCESS;
    while(true) {
      T* curr = head_;
      T* curr_next = head_->next_;
      T* left_next = nullptr;
      left = right = nullptr;
      do
      {
        if (is_unmarked(curr_next)) {
          left = curr;
          left_next = curr_next;
        }
        curr = get_unmark_pointer(curr_next);
        if (curr == tail_)
          break;
        curr_next = curr->next_;
      } while(is_marked(curr_next) || curr->compare(p) < 0);
      right = curr;
      /////////////////////////////////////
      if (left_next == right) {
        if ((right != tail_) && is_marked(right->next_)) {
          continue;
        } else {
          return ret;
        }
      } else {
        ////////////////////////////////////
        if (CAS (&(left->next_), left_next, right)) {
          if ((right != tail_) && is_marked(right->next_)) {
            continue;
          } else {
            return ret;
          }
        }
      }
    }
    return ret;
  }
}

#endif /* YEDIS_LINKED_LIST_LOCKFREE_H_ */
