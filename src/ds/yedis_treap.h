#ifndef YEDIS_TREAP_H_
#define YEDIS_TREAP_H_
#include "../ds/yedis_string.h"
namespace yedis_datastructures
{
  struct YedisTreapNode
  {
  public:
    ~YedisTreapNode()
    {
      if (nullptr != ele) {
        int64_t size_to_free = ele->get_object_size();
        ele->~YedisNormalString();
        yedis_free(ele, size_to_free);
        ele = nullptr;
      }
    }
  public:
    YedisNormalString *ele;
    double score;
    YedisTreapNode *left;
    YedisTreapNode *right;
    uint64_t priority;
    int64_t size;//the node count. including itself.
  };
  //score====>binary search tree
  //priority====>max heap
  class YedisTreap
  {
  public:
    int init();
    ~YedisTreap();
    YedisTreapNode *find(const char *str, const double score);
    YEDIS_MUST_INLINE YedisTreapNode *find(const YedisNormalString *ele, const double score) {return find(ele->get_ptr(), score);}
    int insert(const char *str, const double score);
    int insert(YedisNormalString *ele, const double score);
    int remove(const char *str, const double score);
    int remove(const YedisNormalString *ele, const double score);
    int get_rank(const char *str, const double score);
    YEDIS_MUST_INLINE int get_rank(const YedisNormalString *ele, const double score) {return get_rank(ele->get_ptr(), score);}
    YEDIS_MUST_INLINE int size() {return size_;}
    YedisTreapNode *find_max();
    YedisTreapNode *find_min();
  private:
    void gc_help(YedisTreapNode *p);
    void help_remove(YedisTreapNode *&p);
    int remove(const char *str, const double score, YedisTreapNode *&treap);
    void swap(YedisTreapNode *p, YedisTreapNode *q);
    int insert(YedisNormalString *ele, const double score, YedisTreapNode *&treap);
    YedisTreapNode *left_rotation(YedisTreapNode *k1, YedisTreapNode *k2);
    YedisTreapNode *right_rotation(YedisTreapNode *k1, YedisTreapNode *k2);
    YEDIS_MUST_INLINE static int size(YedisTreapNode *p) {return nullptr == p ? 0 : p->size;}
  private:
    YedisTreapNode *root_;
    YedisTreapNode *sentinel_;
    int64_t size_;
  };
}

#endif /* YEDIS_TREAP_H_ */
