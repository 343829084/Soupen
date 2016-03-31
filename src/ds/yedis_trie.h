#ifndef YEDIS_TRIE_H_
#define YEDIS_TRIE_H_
#include "../base/yedis_common.h"
#include "../server/yedis_global_info.h"
#include <ctype.h>
using yedis_server::dbi;
namespace yedis_datastructures
{
  struct YedisTrieNode
  {
    YedisTrieNode(bool is_case_sensitive)
    {
      int64_t size = is_case_sensitive ? 10 + 26 + 26 + 1 : 10 + 26 + 1;
      next_ = (YedisTrieNode **)yedis_malloc(sizeof(YedisTrieNode*) * size);
      for (int64_t i = 0; i < size; i++) {
        next_[i] = nullptr;
      }
      flag_ = false;
      is_case_sensitive_ = is_case_sensitive;
    }
    ~YedisTrieNode()
    {
      int64_t size = is_case_sensitive_ ? 10 + 26 + 26 + 1 : 10 + 26 + 1;
      yedis_free(next_, sizeof(YedisTrieNode*) * size);
    }
    YedisTrieNode **next_;
    bool flag_;
    bool is_case_sensitive_;
  };
  template<typename T>
  class YedisTrie
  {
  private:
  public:
    YedisTrie(bool is_case_sensitive){root_ = nullptr; is_inited_ = false; is_case_sensitive_ = is_case_sensitive; init();}
    ~YedisTrie();
    int add(const char *p);
    int add(const char *p, const char *q);
    int add(const char *p, const T *ele);
    int add(const char *p, const char *q, const T *ele);
    bool contains(const char *p);
    bool contains(const char *p, const char *q);
    bool contains(const char *p, T* &ele);
    bool contains(const char *p, const char *q, T* &ele);
  private:
    int init();
    bool is_valid_char(const char c, int &id);
    void gc_help(T *p, int64_t size);
  private:
    T *root_;
    bool is_inited_;
    bool is_case_sensitive_;
    int  num_of_chars_;// 63 or 37
    int id_offset_for_lower_case_;
  };

  template<typename T>
  YedisTrie<T>::~YedisTrie()
  {
    int64_t size = is_case_sensitive_ ? 10 + 26 + 26 + 1 : 10 + 26 + 1;
    gc_help(root_, size);
  }

  template<typename T>
  void YedisTrie<T>::gc_help(T *p, int64_t size)
  {
    if (nullptr != p) {
      for (int64_t i = 0; i < size; ++i) {
        gc_help(p->next_[i], size);
      }
      p->~T();
      yedis_free(p, sizeof(T));
    }
  }

  template<typename T>
  int YedisTrie<T>::add(const char *p)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(nullptr == p)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      ret = add(p, p + strlen(p), nullptr);
    }
    return ret;
  }

  template<typename T>
  int YedisTrie<T>::add(const char *p, const char *q)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(nullptr == p)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      ret = add(p, q, nullptr);
    }
    return ret;
  }

  template<typename T>
  int YedisTrie<T>::add(const char *p, const T *ele)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(nullptr == p)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      ret = add(p, p + strlen(p), ele);
    }
    return ret;
  }

  template<typename T>
  YEDIS_MUST_INLINE int YedisTrie<T>::add(const char *p, const char *q, const T *ele)
  {
    int ret = YEDIS_SUCCESS;
    const char *r = p;
    T *tmp = root_;
    int id = 0;
    while(r < q) {
      if (!is_valid_char(*r, id)) {
        ret = YEDIS_ERROR_INVALID_ARGUMENT;
        break;
      } else if (nullptr == tmp->next_[id]) {
        T *buffer = static_cast<T*>(yedis_malloc(sizeof(T)));
        if (YEDIS_UNLIKELY(nullptr == buffer)) {
          ret = YEDIS_ERROR_NO_MEMORY;
          break;
        } else {
          tmp->next_[id] = new (buffer)T(is_case_sensitive_);
        }
      }
      tmp = tmp->next_[id];
      r++;
    } //end while
    if (YEDIS_SUCCESS == ret && r >= q) {
      if(ele != nullptr) {
        *tmp = *ele;
      }
      tmp->flag_ = true;//ok even when *p is '\0'
    }
    return ret;
  }

  template<typename T>
  int YedisTrie<T>::init()
  {
    int ret = YEDIS_SUCCESS;
    T *buffer = static_cast<T*>(yedis_malloc(sizeof(T)));
    if (YEDIS_UNLIKELY(nullptr == buffer)) {
      ret = YEDIS_ERROR_NO_MEMORY;
    } else {
      root_ = new (buffer)T(is_case_sensitive_);
      if (is_case_sensitive_) {
        id_offset_for_lower_case_ = 36;
        num_of_chars_ = 10+26+26+1;
      } else {
        id_offset_for_lower_case_ = 10;
        num_of_chars_ = 10+26+1;
      }
      is_inited_ = true;
    }
    return ret;
  }

  template<typename T>
  bool YedisTrie<T>::is_valid_char(const char c, int &id)
  {
    int ret = true;
    if (c >= '0' && c <= '9') {
      id = 0 + c - '0';
    } else if (c >= 'A' && c <= 'Z') {
      id = 10 + c - 'A';
    } else if ('_'  == c) {
      id = num_of_chars_ - 1;//store '_' in the last position of next_
    } else if (c >= 'a' && c <= 'z') {
      id = id_offset_for_lower_case_ + c - 'a';
    } else {
      ret = false;
    }
    return ret;
  }

  template<typename T>
  bool YedisTrie<T>::contains(const char *p)
  {
    const char *q = p;
    bool ret = true;
    T *tmp = root_;
    int id = 0;
    while(*q != '\0' && nullptr != tmp) {
      if (YEDIS_UNLIKELY(!is_valid_char(*q, id))) {
        ret = false;
        break;
      } else if (nullptr == tmp->next_[id]) {
        ret = false;
        break;
      } else {
        tmp = tmp->next_[id];
      }
      q++;
    }
    if (ret && tmp != nullptr) { //means that *q == '\0'
      ret = tmp->flag_; //if we inserted "abcd" and looking for "abc"
                        //tmp->flag_ will be false. so ret will be false,also.
    }
    return ret;
  }
  template<typename T>
  bool YedisTrie<T>::contains(const char *p, const char *q)
  {
    const char *curr = p;
    bool ret = true;
    T *tmp = root_;
    int id = 0;
    while(curr != q && nullptr != tmp) {
      if (YEDIS_UNLIKELY(!is_valid_char(*curr, id))) {
        ret = false;
        break;
      } else if (nullptr == tmp->next_[id]) {
        ret = false;
        break;
      } else {
        tmp = tmp->next_[id];
      }
      curr++;
    }
    if (ret && tmp != nullptr) {
      ret = tmp->flag_; //if we inserted "abcd" and looking for "abc"
                        //tmp->flag_ will be false. so ret will be false,also.
    }
    return ret;
  }
  template<typename T>
  bool YedisTrie<T>::contains(const char *p, T* &ele)
  {
    const char *q = p;
    bool ret = true;
    T *tmp = root_;
    int id = 0;
    while(*q != '\0' && nullptr != tmp) {
      if (YEDIS_UNLIKELY(!is_valid_char(*q, id))) {
        ret = false;
        break;
      } else if (nullptr == tmp->next_[id]) {
        ret = false;
        break;
      } else {
        tmp = tmp->next_[id];
      }
      q++;
    }
    if (ret && tmp != nullptr) { //means that *q == '\0'
      ret = tmp->flag_; //if we inserted "abcd" and looking for "abc"
                        //tmp->flag_ will be false. so ret will be false,also.
      ele = tmp;
    }
    return ret;
  }
  template<typename T>
  bool YedisTrie<T>::contains(const char *p, const char *q, T* &ele)
  {
    bool ret = true;
    T *tmp = root_;
    int id = 0;
    const char *r = p;
    while(r < q && nullptr != tmp) {
      if (YEDIS_UNLIKELY(!is_valid_char(*r, id))) {
        ret = false;
        break;
      } else if (nullptr == tmp->next_[id]) {
        ret = false;
        break;
      } else {
        tmp = tmp->next_[id];
      }
      r++;
    }
    if (ret && tmp != nullptr) { //means that *q == '\0'
      ret = tmp->flag_; //if we inserted "abcd" and looking for "abc"
                        //tmp->flag_ will be false. so ret will be false,also.
      ele = tmp;
    }
    return ret;
  }
}

#endif /* YEDIS_TRIE_H_ */
