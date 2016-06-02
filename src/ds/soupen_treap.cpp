#include "../ds/soupen_treap.h"
namespace soupen_datastructures
{
  #define INSERT(subtree1, substree2) \
   insert(ele, score, treap->subtree1); \
   if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) { \
     ++treap->size;\
     if (YEDIS_UNLIKELY(treap->subtree1->priority > treap->priority)) { \
       treap = substree2##_rotation(treap, treap->subtree1);\
     }\
   }

  SoupenTreap::~SoupenTreap()
  {
    if (YEDIS_LIKELY(root_ != nullptr)) {
      gc_help(root_);
      root_ = nullptr;
    }
    if (YEDIS_LIKELY(sentinel_ != nullptr)) {
      sentinel_->~SoupenTreapNode();
      soupen_free(sentinel_, sizeof(SoupenTreapNode));
      sentinel_ = nullptr;
    }
  }

  void SoupenTreap::gc_help(SoupenTreapNode *p)
  {
    if (YEDIS_LIKELY(p != sentinel_)) {
      gc_help(p->left);
      gc_help(p->right);
      p->~SoupenTreapNode();
      soupen_free(p, sizeof(SoupenTreapNode));
    }
  }

  int SoupenTreap::init()
  {
    int ret = YEDIS_SUCCESS;
    sentinel_ = root_ = nullptr;
    SoupenTreapNode *tmp = nullptr;
    if (YEDIS_UNLIKELY(nullptr == (tmp = static_cast<SoupenTreapNode*>(soupen_malloc(sizeof(SoupenTreapNode)))))) {
      ret = YEDIS_ERROR_NO_MEMORY;
    } else {
      tmp->ele = nullptr;
      tmp->size = 0;
      tmp->score = 0.0;
      tmp->left = tmp->right = tmp;
      tmp->priority = YEDIS_UINT64_MIN;
      sentinel_ = tmp;
      root_ = sentinel_;//important !
      size_ = 0;//sentinel does not count.
    }
    return 0;
  }

  int SoupenTreap::insert(const char *str, const double score)
  {
    int ret = YEDIS_SUCCESS;
    SoupenString *ele = nullptr;
    if (YEDIS_UNLIKELY(YEDIS_SUCCESS != (ret = SoupenString::factory(str, ele)))) {
      //do nothing
    } else {
      ret = insert(ele, score);
    }
    return ret;
  }

  int SoupenTreap::insert(SoupenString *ele, const double score)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(nullptr == sentinel_)) {
      ret = YEDIS_ERROR_NOT_INITED;
    } else {
      ret = insert(ele, score, root_);
      this->size_ += (YEDIS_SUCCESS == ret);
    }
    return ret;
  }

  int SoupenTreap::insert(SoupenString *ele, const double score, SoupenTreapNode *&treap)
  {
    int ret = YEDIS_SUCCESS;
    int cmp = 0;
    if (treap == sentinel_) {
      if (YEDIS_UNLIKELY(nullptr == (treap = static_cast<SoupenTreapNode*>(soupen_malloc(sizeof(SoupenTreapNode)))))) {
        ret = YEDIS_ERROR_NO_MEMORY;
        LOG_WARN("no memory. allocate failed in redis");
      } else {
        treap->ele = ele;
        treap->size = 1;
        treap->score = score;
        treap->left = treap->right = sentinel_;
        treap->priority = static_cast<uint64_t>(random());
      }
    } else if(treap->score > score) {
      INSERT(left, right);
    } else if (treap->score < score) {
      INSERT(right, left);
    } else if (/*same score and cmp > 0*/(cmp = treap->ele->cmp(ele)) > 0){
      INSERT(left, right);
    } else if (/*same score and cmp < 0*/cmp < 0) {
      INSERT(right, left);
    } else /*same score and cmp = 0*/{
      ret = YEDIS_ERROR_ENTRY_ALREADY_EXISTS;
    }
    return ret;
  }

  SoupenTreapNode *SoupenTreap::left_rotation(SoupenTreapNode *k1, SoupenTreapNode *k2)
  {
    int32_t k1_left_size = size(k1->left);
    int32_t k2_left_size = size(k2->left);
    int32_t k1_size = size(k1);
    k1->right = k2->left;
    k2->left = k1;
    k1->size = k1_left_size + k2_left_size + 1;
    k2->size = k1_size;
    return k2;
  }

  SoupenTreapNode *SoupenTreap::right_rotation(SoupenTreapNode *k1, SoupenTreapNode *k2)
  {
    int32_t k1_right_size = size(k1->right);
    int32_t k2_right_size = size(k2->right);
    int32_t k1_size = size(k1);
    k1->left = k2->right;
    k2->right = k1;
    k1->size = k1_right_size + k2_right_size + 1;
    k2->size = k1_size;
    return k2;
  }

  SoupenTreapNode *SoupenTreap::find(const char *str, const double score)
  {
    SoupenTreapNode *tmp = root_;
    int cmp = 0;
    while(tmp != sentinel_) {
      if (tmp->score > score) {
        tmp = tmp->left;
      } else if (tmp->score < score) {
        tmp = tmp->right;
      } else if (/*same score and cmp = 0*/(cmp = tmp->ele->cmp(str)) == 0) { //found
        break;
      } else if (/*same score and cmp > 0*/cmp < 0) {
        tmp = tmp->right;
      } else {
        tmp = tmp->left;
      }
    }
    return tmp != sentinel_ ? tmp : nullptr;
  }

  int SoupenTreap::get_rank(const char *str, const double score)
  {
    int rank = 1;
    int cmp = 0;
    SoupenTreapNode *tmp = root_;
    while (tmp != sentinel_) {
      if (tmp->score > score) {
        tmp = tmp->left;
      } else if (tmp->score < score) {
        rank += 1 + size(tmp->left);
        tmp = tmp->right;
      } else if (/*same score and cmp = 0*/(cmp = tmp->ele->cmp(str)) == 0) { //found
        return rank + size(tmp->left);
      } else if (/*same score and cmp < 0*/cmp < 0) {
        rank += 1 + size(tmp->left);
        tmp = tmp->right;
      } else /*same score and cmp > 0*/{
        tmp = tmp->left;
      }
    }
    return 0;
  }


  int SoupenTreap::remove(const char *str, const double score)
  {
    return remove(str, score, root_);
  }

  int SoupenTreap::remove(const SoupenString *ele, const double score)
  {
    return remove(ele->get_ptr(), score);
  }

  int SoupenTreap::remove(const char *str, const double score, SoupenTreapNode *&treap)
  {
    int cmp = 0;
    int ret = YEDIS_ERROR_ENTRY_NOT_EXIST;
    if (treap != sentinel_) {
      if (treap->score > score) {
        ret = remove(str, score, treap->left);
        treap->size -= (YEDIS_SUCCESS == ret);
      } else if (treap->score < score) {
        ret = remove(str, score, treap->right);
        treap->size -= (YEDIS_SUCCESS == ret);
      } else if (/*same score and cmp = 0*/(cmp = treap->ele->cmp(str)) > 0) { //found
        ret = remove(str, score, treap->left);
        treap->size -= (YEDIS_SUCCESS == ret);
      } else if (/*same score and cmp > 0*/cmp < 0) {
        ret = remove(str, score, treap->right);
        treap->size -= (YEDIS_SUCCESS == ret);
      } else /*if (cmp == 0)*/ {
        ret = YEDIS_SUCCESS;
        if (treap->left == sentinel_ && treap->right == sentinel_) {
          soupen_reclaim(treap);
        } else if (treap->left->priority > treap->right->priority) {
          treap = right_rotation(treap, treap->left);
          ret = remove(str, score, treap->right);
          treap->size -= (YEDIS_SUCCESS == ret);
        } else {
          treap = left_rotation(treap, treap->right);
          ret = remove(str, score, treap->left);
          treap->size -= (YEDIS_SUCCESS == ret);
        }
      }
    }
    size_ -= (YEDIS_SUCCESS == ret);
    return ret;
  }


  SoupenTreapNode *SoupenTreap::find_min()
  {
    SoupenTreapNode *tmp = root_;
    if (YEDIS_UNLIKELY(sentinel_ == tmp)) {
      return nullptr;
    }
    while(true) {
      if (tmp->left != sentinel_) {
        tmp = tmp->left;
      } else {
        break;
      }
    }
    return tmp;
  }

  SoupenTreapNode *SoupenTreap::find_max()
  {
    SoupenTreapNode *tmp = root_;
    if (YEDIS_UNLIKELY(sentinel_ == tmp)) {
      return nullptr;
    }
    while(true) {
      if (tmp->right != sentinel_) {
        tmp = tmp->right;
      } else {
        break;
      }
    }
    return tmp;
  }
}
