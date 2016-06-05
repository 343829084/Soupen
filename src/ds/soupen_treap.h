#ifndef SOUPEN_TREAP_H_
#define SOUPEN_TREAP_H_
#include "../ds/soupen_string.h"
#include "../lib/soupen_math.h"
namespace soupen_datastructures
{
  struct SoupenTreapNode
  {
  public:
    ~SoupenTreapNode()
    {
      if (nullptr != ele) {
        int64_t size_to_free = ele->get_object_size();
        ele->~SoupenString();
        soupen_free(ele, size_to_free);
        ele = nullptr;
      }
    }
  public:
    SoupenString *ele;
    double score;
    SoupenTreapNode *left;
    SoupenTreapNode *right;
    uint64_t priority;
    int64_t size;//the node count. including itself.
  };
  //score====>binary search tree
  //priority====>max heap
  class SoupenTreap
  {
  public:
    int init();
    ~SoupenTreap();
    SoupenTreapNode *find(const char *str, const double score);
    SOUPEN_MUST_INLINE SoupenTreapNode *find(const SoupenString *ele, const double score) {return find(ele->get_ptr(), score);}
    int insert(const char *str, const double score);
    int insert(SoupenString *ele, const double score);
    int remove(const char *str, const double score);
    int remove(const SoupenString *ele, const double score);
    int get_rank(const char *str, const double score);
    SOUPEN_MUST_INLINE int get_rank(const SoupenString *ele, const double score) {return get_rank(ele->get_ptr(), score);}
    SOUPEN_MUST_INLINE int size() {return size_;}
    SoupenTreapNode *find_max();
    SoupenTreapNode *find_min();
  private:
    void gc_help(SoupenTreapNode *p);
    int remove(const char *str, const double score, SoupenTreapNode *&treap);
    int insert(SoupenString *ele, const double score, SoupenTreapNode *&treap);
    SoupenTreapNode *left_rotation(SoupenTreapNode *k1, SoupenTreapNode *k2);
    SoupenTreapNode *right_rotation(SoupenTreapNode *k1, SoupenTreapNode *k2);
    SOUPEN_MUST_INLINE static int size(SoupenTreapNode *p) {return p->size;}
  private:
    soupen_lib::SoupenRandomGenerator random_;
    SoupenTreapNode *root_;
    SoupenTreapNode *sentinel_;
    int64_t size_;
  };
}

#endif /* SOUPEN_TREAP_H_ */
