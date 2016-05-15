#include "../ds/yedis_cuckoo_hashmap.h"
#include<iostream>
#include<thread>
#include <sys/time.h>
#include <map>
#include <unordered_map>
using namespace std;
using namespace yedis_datastructures;
const uint32_t BASE = 100000000;
struct Node
{
private:
  int64_t n;
};
void test_cuckoo(uint32_t num, double found_percent)
{
  YedisCukooHashMap<int64_t, Node, 12582917, 2> h;
  Node n;
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  for (uint32_t i = BASE; i < BASE + num; i++) {
    h.set(i,n);
  }
  uint32_t start = BASE + num - static_cast<uint32_t>(num * found_percent);
  uint32_t end = BASE + num + (1- found_percent) * num;
  cout<<"start=="<<start<<",end=="<<end<<endl;
  gettimeofday(&t2, NULL);
  //cout<<"found::==>cuckoo==>set"<<(t2.tv_sec-t1.tv_sec) * 1000000 + t2.tv_usec-t1.tv_usec<<endl;
  gettimeofday(&t1, NULL);
  for (uint32_t i = start; i < end; i++) {
    if(h.contains(i)) {

    }
  }
  gettimeofday(&t2, NULL);
  cout<<"percent "<<found_percent * 100 <<"% found. In cuckoo hashmap, get takes "<<(t2.tv_sec-t1.tv_sec) * 1000000 + t2.tv_usec-t1.tv_usec<<endl;
}
void test_std(uint32_t num, double found_percent)
{
  unordered_map<int64_t, Node> mymap;
  Node n;
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  for (uint32_t i = BASE; i < BASE + num; i++) {
    mymap[i]= n;
  }
  gettimeofday(&t2, NULL);
  uint32_t start = BASE + num - static_cast<uint32_t>(num * found_percent);
  uint32_t end = BASE + num + (1- found_percent) * num;
  cout<<"start=="<<start<<",end=="<<end<<endl;
  //cout<<"found::==>unordered map==>set"<<(t2.tv_sec-t1.tv_sec) * 1000000 + t2.tv_usec-t1.tv_usec<<endl;
  gettimeofday(&t1, NULL);
  for (uint32_t i = start; i < end; i++) {
    if(mymap.find(i) != mymap.end()){
    }
  }
  gettimeofday(&t2, NULL);
  cout<<"percent "<<found_percent * 100 <<"% found. In unordered map, get takes "<<(t2.tv_sec-t1.tv_sec) * 1000000 + t2.tv_usec-t1.tv_usec<<endl;
}
void test(uint32_t num, double found_percent)
{
  test_cuckoo(num, found_percent);
  test_std(num, found_percent);
}
int main()
{
  //test(3900000, 0.1);
  //test(3900000, 0.8);
  test(18000000, 1);
  return 0;
}
