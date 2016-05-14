#include "../ds/yedis_treap.h"
namespace yedis_datastructures
{
  using namespace yedis_server;
  #define INSERT(subtree1, substree2) \
   insert(ele, score, treap->subtree1); \
   if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) { \
     ++treap->size;\
     if (YEDIS_UNLIKELY(treap->subtree1->priority > treap->priority)) { \
       treap = substree2##_rotation(treap, treap->subtree1);\
     }\
   }

  YedisTreap::~YedisTreap()
  {
    if (YEDIS_LIKELY(root_ != nullptr)) {
      gc_help(root_);
      root_ = nullptr;
    }
    if (YEDIS_LIKELY(sentinel_ != nullptr)) {
      sentinel_->~YedisTreapNode();
      yedis_free(sentinel_, sizeof(YedisTreapNode));
      sentinel_ = nullptr;
    }
  }

  void YedisTreap::gc_help(YedisTreapNode *p)
  {
    if (YEDIS_LIKELY(p != sentinel_)) {
      gc_help(p->left);
      gc_help(p->right);
      p->~YedisTreapNode();
      yedis_free(p, sizeof(YedisTreapNode));
    }
  }

  int YedisTreap::init()
  {
    int ret = YEDIS_SUCCESS;
    sentinel_ = root_ = nullptr;
    YedisTreapNode *tmp = nullptr;
    if (YEDIS_UNLIKELY(nullptr == (tmp = static_cast<YedisTreapNode*>(yedis_malloc(sizeof(YedisTreapNode)))))) {
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

  int YedisTreap::insert(const char *str, const double score)
  {
    int ret = YEDIS_SUCCESS;
    YedisNormalString *ele = nullptr;
    if (YEDIS_UNLIKELY(YEDIS_SUCCESS != (ret = YedisNormalString::factory(str, ele)))) {
      //do nothing
    } else {
      ret = insert(ele, score);
    }
    return ret;
  }

  int YedisTreap::insert(YedisNormalString *ele, const double score)
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

  int YedisTreap::insert(YedisNormalString *ele, const double score, YedisTreapNode *&treap)
  {
    int ret = YEDIS_SUCCESS;
    int cmp = 0;
    if (treap == sentinel_) {
      if (YEDIS_UNLIKELY(nullptr == (treap = static_cast<YedisTreapNode*>(yedis_malloc(sizeof(YedisTreapNode)))))) {
        ret = YEDIS_ERROR_NO_MEMORY;
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

  YedisTreapNode *YedisTreap::left_rotation(YedisTreapNode *k1, YedisTreapNode *k2)
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

  YedisTreapNode *YedisTreap::right_rotation(YedisTreapNode *k1, YedisTreapNode *k2)
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

  YedisTreapNode *YedisTreap::find(const char *str, const double score)
  {
    YedisTreapNode *tmp = root_;
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
    return tmp;
  }

  int YedisTreap::get_rank(const char *str, const double score)
  {
    int rank = 1;
    int cmp = 0;
    YedisTreapNode *tmp = root_;
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

  YedisTreapNode *YedisTreap::find_min()
  {
    YedisTreapNode *tmp = root_;
    if (YEDIS_UNLIKELY(sentinel_ == tmp)) {
      return tmp;
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

  YedisTreapNode *YedisTreap::find_max()
  {
    YedisTreapNode *tmp = root_;
    if (YEDIS_UNLIKELY(sentinel_ == tmp)) {
      return tmp;
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

  void YedisTreap::swap(YedisTreapNode *p, YedisTreapNode *q)
  {
    YedisNormalString *ele = p->ele;
    double score = p->score;
    uint64_t priority = p->priority;

    p->ele = q->ele;
    p->score = q->score;
    p->priority = q->priority;

    q->ele = ele;
    q->score = score;
    q->priority = priority;
  }
}
