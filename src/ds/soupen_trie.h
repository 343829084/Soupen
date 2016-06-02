#ifndef YEDIS_TRIE_H_
#define YEDIS_TRIE_H_
#include "../base/soupen_memory.h"
#include <ctype.h>
namespace soupen_datastructures
{
  struct SoupenTrieNode
  {
    int init(bool is_case_sensitive)
    {
      int ret = YEDIS_SUCCESS;
      is_inited_ = false;
      int64_t size = is_case_sensitive ? 10 + 26 + 26 + 1 : 10 + 26 + 1;
      next_ = (SoupenTrieNode **)soupen_malloc(sizeof(SoupenTrieNode*) * size);
      if (YEDIS_UNLIKELY(nullptr == next_)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        for (int64_t i = 0; i < size; i++) {
          next_[i] = nullptr;
        }
        flag_ = false;
        is_case_sensitive_ = is_case_sensitive;
        is_inited_ = true;
      }
      return ret;
    }
    ~SoupenTrieNode()
    {
      int64_t size = is_case_sensitive_ ? 10 + 26 + 26 + 1 : 10 + 26 + 1;
      soupen_free(next_, sizeof(SoupenTrieNode*) * size);
      is_inited_ = false;
    }
    bool is_inited() {return is_inited_;}
    SoupenTrieNode **next_;
    bool flag_;
    bool is_case_sensitive_;
    bool is_inited_;
  };
  template<typename T>
  class SoupenTrie
  {
  private:
  public:
    SoupenTrie(bool is_case_sensitive) { init(is_case_sensitive);}
    ~SoupenTrie();
    YEDIS_MUST_INLINE int add(const char *p);
    YEDIS_MUST_INLINE int add(const char *p, const char *q);
    YEDIS_MUST_INLINE int add(const char *p, const T *ele);
    int add(const char *p, const char *q, const T *ele);
    YEDIS_MUST_INLINE bool contains(const char *p);
    YEDIS_MUST_INLINE bool contains(const char *p, const char *q);
    YEDIS_MUST_INLINE bool contains(const char *p, T* &ele);
    bool contains(const char *p, const char *q, T* &ele);
    int init(bool is_case_sensitive);
    bool is_inited() {return is_inited_;}
  private:
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
  SoupenTrie<T>::~SoupenTrie()
  {
    int64_t size = is_case_sensitive_ ? 10 + 26 + 26 + 1 : 10 + 26 + 1;
    gc_help(root_, size);
    is_inited_ = false;
  }

  template<typename T>
  void SoupenTrie<T>::gc_help(T *p, int64_t size)
  {
    if (nullptr != p) {
      for (int64_t i = 0; i < size; ++i) {
        gc_help(p->next_[i], size);
      }
      soupen_reclaim(p);
    }
  }

  template<typename T>
  int SoupenTrie<T>::add(const char *p)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited())) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(nullptr == p)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      ret = add(p, p + strlen(p), nullptr);
    }
    return ret;
  }

  template<typename T>
  int SoupenTrie<T>::add(const char *p, const char *q)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited())) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(nullptr == p || nullptr == q)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      ret = add(p, q, nullptr);
    }
    return ret;
  }

  template<typename T>
  int SoupenTrie<T>::add(const char *p, const T *ele)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(!is_inited())) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else if (YEDIS_UNLIKELY(nullptr == p)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      ret = add(p, p + strlen(p), ele);
    }
    return ret;
  }

  template<typename T>
  YEDIS_MUST_INLINE int SoupenTrie<T>::add(const char *p, const char *q, const T *ele)
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
        T *buffer = static_cast<T*>(soupen_malloc(sizeof(T)));
        if (YEDIS_UNLIKELY(nullptr == buffer)) {
          ret = YEDIS_ERROR_NO_MEMORY;
          break;
        } else if (YEDIS_UNLIKELY(YEDIS_SUCCESS != (ret = buffer->init(is_case_sensitive_)))) {
          soupen_reclaim(buffer);
        } else {
          tmp->next_[id] = buffer;
        }
      }
      tmp = tmp->next_[id];
      r++;
    } //end while
    if (YEDIS_SUCCED && r >= q) {
      if(ele != nullptr) {
        *tmp = *ele;
      }
      tmp->flag_ = true;//ok even when *p is '\0'
    }
    return ret;
  }

  template<typename T>
  int SoupenTrie<T>::init(bool is_case_sensitive)
  {
    int ret = YEDIS_SUCCESS;
    root_ = nullptr;
    is_inited_ = false;
    is_case_sensitive_ = is_case_sensitive;
    T *buffer = static_cast<T*>(soupen_malloc(sizeof(T)));
    if (YEDIS_UNLIKELY(nullptr == buffer)) {
      ret = YEDIS_ERROR_NO_MEMORY;
    } else if (YEDIS_UNLIKELY(YEDIS_SUCCESS != (ret = buffer->init(is_case_sensitive_)))) {
      soupen_reclaim(buffer);
    } else {
      root_ = buffer;
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
  bool SoupenTrie<T>::is_valid_char(const char c, int &id)
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
  YEDIS_MUST_INLINE bool SoupenTrie<T>::contains(const char *p)
  {
    bool ret = false;
    if (YEDIS_LIKELY(nullptr != p)) {
      T *tmp = nullptr;
      const char *q = p + strlen(p);
      ret = contains(p, q, tmp);
    }
  }
  template<typename T>
  YEDIS_MUST_INLINE bool SoupenTrie<T>::contains(const char *p, const char *q)
  {
    bool ret = false;
    if (YEDIS_LIKELY(nullptr != p && nullptr != q)) {
      T *tmp = nullptr;
      ret = contains(p, q, tmp);
    }
    return ret;
  }
  template<typename T>
  YEDIS_MUST_INLINE bool SoupenTrie<T>::contains(const char *p, T* &ele)
  {
    bool ret = false;
    if (YEDIS_LIKELY(nullptr != p)) {
      const char *q = p + strlen(p);
      ret = contains(p, q, ele);
    }
    return ret;
  }
  template<typename T>
  bool SoupenTrie<T>::contains(const char *p, const char *q, T* &ele)
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
    if (ret && tmp != nullptr) {
      ret = tmp->flag_; //if we inserted "abcd" and looking for "abc"
                        //tmp->flag_ will be false. so ret will be false,also.
      ele = tmp;
    }
    return ret;
  }
}

#endif /* YEDIS_TRIE_H_ */
