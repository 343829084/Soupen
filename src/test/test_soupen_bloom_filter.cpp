#include "../ds/soupen_string.h"
#include "../ds/soupen_bloom_filter.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace soupen_datastructures;
void bf_string_test()
{
  SoupenString ys("helloworld");
  ys.append("123456789999999999999999999999999999999999");
  ys.append("abc");
  ys.append("123456789999999999999999999999999999999999");
  SoupenBloomFilter bf(1000,10);
  bf.add("helloworld");
}
int main()
{
  bf_string_test();
  //cout<<dbi.soupen_total_memory_used<<endl;
  return 0;
}
