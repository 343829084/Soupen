#include<stdio.h>
#include<stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "skiplist.h"
using namespace std;
#include <sys/time.h>
#define N 1000000
int64_t redis_insert_cost = 0;
int64_t redis_rank_cost = 0;
void test_skip_perf(bool is_random)
{
  srand(time(0));
  int i;
  int *key = (int*) malloc(N * sizeof(int));
  vector<int> tmp;
  for (i = 0; i < N; i++) {
    tmp.push_back(i);
  }
  if (is_random) {
    std::random_shuffle(tmp.begin(),tmp.end());
  }
  for (i = 0; i < N; i++) {
    key[i] = tmp[i];
  }
  robj **obj = (robj**) malloc(N * sizeof(robj*));
  char c[20];
  zskiplist *p = zslCreate();
  struct timeval start;
  struct timeval finish;
  gettimeofday(&start,NULL);
  for (int i = 0; i < N; i++) {
    sprintf(c, "%d", key[i]);
    obj[i] = createRawStringObject(c, strlen(c));
    zslInsert(p, key[i], obj[i]);
  }
  gettimeofday(&finish,NULL);
  redis_insert_cost += (finish.tv_sec - start.tv_sec)*1000000LL + (finish.tv_usec-start.tv_usec);
  gettimeofday(&start,NULL);
  for (i = 0; i < N; i++) {
    int rank = zslGetRank(p, key[i], obj[i]);
    if (rank <= 0 ) cout<<"rank...so bad"<<endl;
  }
  gettimeofday(&finish,NULL);
  redis_rank_cost += (finish.tv_sec - start.tv_sec)*1000000LL + (finish.tv_usec-start.tv_usec);
}
int redis_main()
{
  for (int i = 0; i < 10; i++) {
    test_skip_perf(false);
  }
  cout<<"ordered redis rank cost"<<redis_rank_cost<<endl;
  cout<<"ordered redis insert cost"<<redis_insert_cost<<endl;
  redis_rank_cost = redis_insert_cost = 0;
  for (int i = 0; i < 10; i++) {
    test_skip_perf(true);
  }
  cout<<"random redis rank cost"<<redis_rank_cost<<endl;
  cout<<"random redis insert cost"<<redis_insert_cost<<endl;
  return 0;
}
