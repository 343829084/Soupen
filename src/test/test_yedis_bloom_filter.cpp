#include "../ds/yedis_string.h"
#include "../ds/yedis_bloom_filter.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace yedis_datastructures;
void bf_string_test()
{
  YedisString ys("helloworld");
  ys.append("123456789999999999999999999999999999999999");
  ys.append("abc");
  ys.append("123456789999999999999999999999999999999999");
  YedisBloomFilter bf(1000,10);
  bf.add("helloworld");
}
int main()
{
  bf_string_test();
  //cout<<dbi.yedis_total_memory_used<<endl;
  return 0;
}
