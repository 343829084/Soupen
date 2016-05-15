#include "../ds/yedis_treap.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <sys/time.h>
using namespace std;
int N = 1;
using namespace yedis_datastructures;
int64_t yedis_insert_cost = 0;
int64_t yedis_rank_cost = 0;
void test_treap_perf(bool is_random)
{
  N = 1000000;
  int i;
  int *key = (int*) malloc(N * sizeof(int));
  vector<int> tmp;
  char c[20];
  YedisNormalString **str = (YedisNormalString**) malloc(N * sizeof(YedisNormalString*));
  for (i = 0; i < N; i++) {
    tmp.push_back(i + 1);
  }
  srand(time(0));
  if (is_random) {
    std::random_shuffle(tmp.begin(),tmp.end());
  }
  for (i = 0; i < N; i++) {
    key[i] = tmp[i];
  }
  YedisTreap *p = (YedisTreap *)yedis_malloc(sizeof(YedisTreap));
  p->init();
  timespec t1, t2;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  for (i = 0; i < N; i++) {
    sprintf(c, "%d", key[i]);
    YedisNormalString::factory(c, str[i]);
    str[i]->init(c);
    p->insert(str[i], key[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &t2);
  yedis_insert_cost += (t2.tv_sec - t1.tv_sec) * 1000000000LL + t2.tv_nsec - t1.tv_nsec;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  for (i = 0; i < N; i++) {
    int rank = p->get_rank(str[i], key[i]);
    if (rank <= 0) {cout<<"rank bad  "<<N<<endl;}
  }
  clock_gettime(CLOCK_MONOTONIC, &t2);
  yedis_rank_cost += (t2.tv_sec - t1.tv_sec) * 1000000000LL + t2.tv_nsec - t1.tv_nsec;
  p->~YedisTreap();
  yedis_free(p, sizeof(YedisTreap));
  free(key);
  free(str);
}

void test_treap_correct()
{
  int i;
  int *key = (int*) malloc(N * sizeof(int));
  vector<int> tmp;
  char c[20];
  YedisNormalString **str = (YedisNormalString**) malloc(N * sizeof(YedisNormalString*));
  for (i = 0; i < N; i++) {
    tmp.push_back(i + 1);
  }
  srand(time(0));
  std::random_shuffle(tmp.begin(),tmp.end());
  for (i = 0; i < N; i++) {
    key[i] = tmp[i];
  }
  YedisTreap *p = (YedisTreap *)yedis_malloc(sizeof(YedisTreap));
  p->init();
  //insert
  for (i = 0; i < N; i++) {
    sprintf(c, "%d", key[i]);
    int ret = YedisNormalString::factory(c, str[i]);
    YEDIS_ASSERT(ret == 0);
    ret = str[i]->init(c);
    YEDIS_ASSERT(ret == 0);
    ret = p->insert(str[i], key[i]);
    YEDIS_ASSERT(ret == 0);
  }
  //size
  int size = p->size();
  YEDIS_ASSERT(size == N);
  //min max
  YedisTreapNode *pmin = p->find_min();
  YEDIS_ASSERT(pmin != nullptr);
  YEDIS_ASSERT(pmin->score == 1);
  YedisTreapNode *pmax = p->find_max();
  YEDIS_ASSERT(pmax != nullptr);
  YEDIS_ASSERT(pmax->score == N);
  //find
  for (i = 0; i < N; i++) {
    YedisTreapNode *res = p->find(str[i], key[i]);
    YEDIS_ASSERT(res != nullptr);
    YEDIS_ASSERT(res->score == key[i]);
  }
  //rank
  for (i = 0; i < N; i++) {
    int realrank = 1;
    for (int j = 0; j < N;j++) {
      realrank += key[j] < key[i];
    }
    int r = p->get_rank(str[i], key[i]);
    YEDIS_ASSERT(realrank == r);
  }
  p->~YedisTreap();
  yedis_free(p, sizeof(YedisTreap));
  free(key);
  free(str);
}

void test_treap_correct2()
{
  const int N = 10;
  int i;
  int key[N]= {6,5,10,8,1,9,2,3,4,7};
  char c[20];
  YedisNormalString *str[3 * N + 5] = {nullptr};
  YedisTreap *p = (YedisTreap *)yedis_malloc(sizeof(YedisTreap));
  p->init();
  //insert
  for (i = 0; i < 3 * N + 5; i++) {
    sprintf(c, "%d", i);
    int ret = YedisNormalString::factory(c, str[i]);
    YEDIS_ASSERT(ret == 0);
    ret = str[i]->init(c);
    YEDIS_ASSERT(ret == 0);
    ret = p->insert(str[i], key[i % N]);
    YEDIS_ASSERT(ret == 0);
  }
  //size
  int size = p->size();
  YEDIS_ASSERT(size == 3 * N + 5);
  //min max
  YedisTreapNode *pmin = p->find_min();
  YEDIS_ASSERT(pmin != nullptr);
  YEDIS_ASSERT(pmin->score == 1);
  YEDIS_ASSERT(strcmp(pmin->ele->get_ptr(), "4") == 0);
  YedisTreapNode *pmax = p->find_max();
  YEDIS_ASSERT(pmax != nullptr);
  YEDIS_ASSERT(pmax->score == 10);
  YEDIS_ASSERT(strcmp(pmax->ele->get_ptr(), "12") == 0);
  //find
  for (i = 0; i < 3 * N + 5; i++) {
    YedisTreapNode *res = p->find(str[i], key[i % N]);
    YEDIS_ASSERT(res != nullptr);
    YEDIS_ASSERT(res->score == key[i % N]);
  }
  p->~YedisTreap();
  yedis_free(p, sizeof(YedisTreap));
}

//treap = substree2##_rotation(treap, treap->subtree1);

int main()
{
  test_treap_correct2();
  for (int i = 0; i < 10; i++) {
      test_treap_perf(false);
  }
  cout<<"ordered : yedis rank cost"<<yedis_rank_cost<<endl;
  cout<<"ordered : yedis insert cost"<<yedis_insert_cost<<endl;
  yedis_rank_cost = yedis_insert_cost = 0;
  for (int i = 0; i < 10; i++) {
    test_treap_perf(true);
  }
  cout<<"random : yedis rank cost"<<yedis_rank_cost<<endl;
  cout<<"random : yedis insert cost"<<yedis_insert_cost<<endl;
  for (int i = 1; i <= 2000;i++) {
    N = i;
    test_treap_correct();
  }
  cout<<"correct"<<endl;
  return 0;
}
