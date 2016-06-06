#include "../ds/soupen_treap.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <sys/time.h>
using namespace std;
int N = 1;
using namespace soupen_datastructures;
int64_t soupen_insert_cost = 0;
int64_t soupen_rank_cost = 0;
void test_treap_perf(bool is_random)
{
  N = 1000000;
  int i;
  int *key = (int*) malloc(N * sizeof(int));
  vector<int> tmp;
  char c[20];
  SoupenString **str = (SoupenString**) malloc(N * sizeof(SoupenString*));
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
  SoupenTreap *p = (SoupenTreap *)soupen_malloc(sizeof(SoupenTreap));
  p->init();
  timespec t1, t2;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  for (i = 0; i < N; i++) {
    sprintf(c, "%d", key[i]);
    SoupenString::factory(c, str[i]);
    str[i]->init(c);
    p->insert(str[i], key[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &t2);
  soupen_insert_cost += (t2.tv_sec - t1.tv_sec) * 1000000000LL + t2.tv_nsec - t1.tv_nsec;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  for (i = 0; i < N; i++) {
    int rank = p->get_rank(str[i], key[i]);
    if (rank <= 0) {cout<<"rank bad  "<<N<<endl;}
  }
  clock_gettime(CLOCK_MONOTONIC, &t2);
  soupen_rank_cost += (t2.tv_sec - t1.tv_sec) * 1000000000LL + t2.tv_nsec - t1.tv_nsec;
  p->~SoupenTreap();
  soupen_free(p, sizeof(SoupenTreap));
  free(key);
  free(str);
}

void test_treap_correct()
{
  int i;
  int *key = (int*) malloc(N * sizeof(int));
  vector<int> tmp;
  char c[20];
  SoupenString **str = (SoupenString**) malloc(N * sizeof(SoupenString*));
  for (i = 0; i < N; i++) {
    tmp.push_back(i + 1);
  }
  srand(time(0));
  std::random_shuffle(tmp.begin(),tmp.end());
  for (i = 0; i < N; i++) {
    key[i] = tmp[i];
  }
  SoupenTreap *p = (SoupenTreap *)soupen_malloc(sizeof(SoupenTreap));
  p->init();
  //insert
  for (i = 0; i < N; i++) {
    sprintf(c, "%d", key[i]);
    int ret = SoupenString::factory(c, str[i]);
    SOUPEN_ASSERT(ret == 0);
    ret = str[i]->init(c);
    SOUPEN_ASSERT(ret == 0);
    ret = p->insert(str[i], key[i]);
    SOUPEN_ASSERT(ret == 0);
  }
  //size
  int size = p->size();
  SOUPEN_ASSERT(size == N);
  //min max
  SoupenTreapNode *pmin = p->find_min();
  SOUPEN_ASSERT(pmin != nullptr);
  SOUPEN_ASSERT(pmin->score == 1);
  SoupenTreapNode *pmax = p->find_max();
  SOUPEN_ASSERT(pmax != nullptr);
  SOUPEN_ASSERT(pmax->score == N);
  //find
  for (i = 0; i < N; i++) {
    SoupenTreapNode *res = p->find(str[i], key[i]);
    SOUPEN_ASSERT(res != nullptr);
    SOUPEN_ASSERT(res->score == key[i]);
  }
  //rank
  for (i = 0; i < N; i++) {
    int realrank = 1;
    for (int j = 0; j < N;j++) {
      realrank += key[j] < key[i];
    }
    int r = p->get_rank(str[i], key[i]);
    SOUPEN_ASSERT(realrank == r);
  }
  p->~SoupenTreap();
  soupen_free(p, sizeof(SoupenTreap));
  free(key);
  free(str);
}

void test_treap_correct2()
{
  const int N = 10;
  int i;
  int key[N]= {6,5,10,8,1,9,2,3,4,7};
  char c[20];
  SoupenString *str[3 * N + 5] = {nullptr};
  SoupenTreap *p = (SoupenTreap *)soupen_malloc(sizeof(SoupenTreap));
  p->init();
  //insert
  for (i = 0; i < 3 * N + 5; i++) {
    sprintf(c, "%d", i);
    int ret = SoupenString::factory(c, str[i]);
    SOUPEN_ASSERT(ret == 0);
    ret = str[i]->init(c);
    SOUPEN_ASSERT(ret == 0);
    ret = p->insert(str[i], key[i % N]);
    SOUPEN_ASSERT(ret == 0);
  }
  //size
  int size = p->size();
  SOUPEN_ASSERT(size == 3 * N + 5);
  //min max
  SoupenTreapNode *pmin = p->find_min();
  SOUPEN_ASSERT(pmin != nullptr);
  SOUPEN_ASSERT(pmin->score == 1);
  SOUPEN_ASSERT(strcmp(pmin->ele->get_ptr(), "4") == 0);
  SoupenTreapNode *pmax = p->find_max();
  SOUPEN_ASSERT(pmax != nullptr);
  SOUPEN_ASSERT(pmax->score == 10);
  SOUPEN_ASSERT(strcmp(pmax->ele->get_ptr(), "12") == 0);
  //find
  for (i = 0; i < 3 * N + 5; i++) {
    SoupenTreapNode *res = p->find(str[i], key[i % N]);
    SOUPEN_ASSERT(res != nullptr);
    SOUPEN_ASSERT(res->score == key[i % N]);
  }
  p->~SoupenTreap();
  soupen_free(p, sizeof(SoupenTreap));
}

void test_treap_correct3(int ele_to_delete)
{
  const int N = 10;
  int i;
  int key[N]= {6,5,10,8,1,9,2,3,4,7};
  int rank[N] = {0};
  char c[20];
  SoupenString *str[N] = {nullptr};
  SoupenTreap *p = (SoupenTreap *)soupen_malloc(sizeof(SoupenTreap));
  p->init();
  //insert
  for (i = 0; i < N; i++) {
    sprintf(c, "%d", i);
    int ret = SoupenString::factory(c, str[i]);
    SOUPEN_ASSERT(ret == 0);
    ret = str[i]->init(c);
    SOUPEN_ASSERT(ret == 0);
    ret = p->insert(str[i], key[i]);
    SOUPEN_ASSERT(ret == 0);
  }
  for (i = 0; i < N; i++) {
    rank[i] = p->get_rank(str[i], key[i]);
  }
  //find
  for (i = 0; i < N; i++) {
    if (key[i] == ele_to_delete) {
      break;
    }
  }
  //del
  int ret = p->remove(str[i], key[i]);
  SOUPEN_ASSERT(ret == 0);
  for (int j = 0; j < N; ++j) {
    if (j != i) {
      SOUPEN_ASSERT(p->find(str[j], key[j]) != nullptr);
    }
  }
  SoupenString *tmp = nullptr;
  sprintf(c, "%d", i);
  ret = SoupenString::factory(c, tmp);
  SOUPEN_ASSERT(ret == 0);
  ret = tmp->init(c);
  SOUPEN_ASSERT(ret == 0);
  SoupenTreapNode *res = p->find(tmp, key[i]);
  SOUPEN_ASSERT(res == nullptr);
  for (int j = 0; j < N; ++j) {
    if (key[j] < ele_to_delete) {
      SOUPEN_ASSERT(p->get_rank(str[j], key[j]) == rank[j]);
    } else if (key[j] == ele_to_delete) {
    } else {
      SOUPEN_ASSERT(p->get_rank(str[j], key[j]) == rank[j] - 1);
    }
  }
  p->~SoupenTreap();
  soupen_free(p, sizeof(SoupenTreap));
}

int main()
{
  test_treap_correct2();
  for (int i = 0; i < 10; i++) {
      test_treap_perf(false);
  }
  for (int i = 1; i <= 10; i++) {
    test_treap_correct3(i);
  }
  cout<<"ordered : soupen rank cost"<<soupen_rank_cost<<endl;
  cout<<"ordered : soupen insert cost"<<soupen_insert_cost<<endl;
  soupen_rank_cost = soupen_insert_cost = 0;
  for (int i = 0; i < 10; i++) {
    test_treap_perf(true);
  }
  cout<<"random : soupen rank cost"<<soupen_rank_cost<<endl;
  cout<<"random : soupen insert cost"<<soupen_insert_cost<<endl;
  for (int i = 1; i <= 2000;i++) {
    N = i;
    test_treap_correct();
  }
  cout<<"correct"<<endl;
  return 0;
}
