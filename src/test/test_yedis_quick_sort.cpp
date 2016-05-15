#include<iostream>
#include<algorithm>
#include<vector>
#include<time.h>
#include"../algorithms/yedis_quick_sort.h"
using namespace std;
typedef MyCompareOperator<int> CO; //用于YedisSort
int cmp ( const void *a , const void *b ) { return *(int *)a - *(int *)b; }//用于qsort
//随机的1000W元素
int test1()
{
  const int SIZE = 10000000;
  vector<int> vi;
  for (int i = 0; i < SIZE; i++)
    vi.push_back(i);
  random_shuffle(vi.begin(), vi.end());

  int *a = new int[SIZE];
  int *b = new int[SIZE];
  int *c = new int[SIZE];
  for (int i =0; i < SIZE;i++) {
    a[i] = vi[i];
    b[i] = vi[i];
    c[i] = vi[i];
  }
  timespec t1, t2;
  unsigned long long cost = 0;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sort(b, b+SIZE);
  clock_gettime(CLOCK_MONOTONIC, &t2);
  cout<<(t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec<<endl;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  YedisSort<int, CO, 30>::sort(a, SIZE);
  clock_gettime(CLOCK_MONOTONIC, &t2);
  cout<<(t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec<<endl;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  qsort(c,SIZE,sizeof(c[0]),cmp);
  clock_gettime(CLOCK_MONOTONIC, &t2);
  cout<<(t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec<<endl;
  delete []a;
  delete []b;
  delete []c;
}

//1000W元素都相同
int test2()
{
  const int SIZE = 10000000;
  vector<int> vi;
  for (int i = 0; i < SIZE; i++)
    vi.push_back(i);
  random_shuffle(vi.begin(), vi.end());
  int *a = new int[SIZE];
  int *b = new int[SIZE];
  int *c = new int[SIZE];
  for (int i =0; i < SIZE;i++) {
    a[i] = 11;
    b[i] = 11;
    c[i] = 11;
  }
  timespec t1, t2;
  unsigned long long cost = 0;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sort(b, b+SIZE);
  clock_gettime(CLOCK_MONOTONIC, &t2);
  cost = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec;
  cout<<cost<<endl;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  YedisSort<int, CO, 30>::sort(a, SIZE);
  clock_gettime(CLOCK_MONOTONIC, &t2);
  cost = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec;
  cout<<cost<<endl;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  qsort(c,SIZE,sizeof(c[0]),cmp);
  clock_gettime(CLOCK_MONOTONIC, &t2);
  cost = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec;
  cout<<cost<<endl;
  delete []a;
  delete []b;
  delete []c;
}
int main()
{
  test1();
  test2();
  return 0;
}

