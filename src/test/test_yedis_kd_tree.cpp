#include "../ds/yedis_kd_tree.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace yedis_datastructures;

typedef YedisKDTreeNode<int, 3> KDTreeNode;

template<typename T>
class CallBack
{
public:
  void operator() (T *p)
  {
    delete p;
    cout<<"delete:"<<p<<endl;
  }
};

void test_kdtree()
{
  KDTreeNode *p = new KDTreeNode();
  KDTreeNode *q = new KDTreeNode();
  p->set(0, 100);
  p->set(1, 200);
  p->set(2, 300);

  q->set(0, 600);
  q->set(1, 700);
  q->set(2, 800);

  KDTreeNode lower;
  lower.set(0, 11);
  lower.set(1, 22);
  lower.set(2, 33);

  KDTreeNode upper;
  upper.set(0, 400);
  upper.set(1, 300);
  upper.set(2, 500);

  YedisKDtree<KDTreeNode, 3, CallBack<KDTreeNode> > tree;
  tree.insert(p);
  tree.insert(q);

  tree.query_range(lower, upper);
}
int main()
{
  test_kdtree();
  return 0;
}
