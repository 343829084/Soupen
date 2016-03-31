#include "../ds/yedis_array.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace yedis_datastructures;
struct Student
{
  int a;
  int b;
};
void yedis_array_test1()
{
  Student s;
  YedisArray<Student, 10> ya;
  for (int i = 2; i <= 97; i++) {
    s.a = i;
    s.b = i+1;
    ya.push_back(s);
  }
  cout<<ya.get_size()<<endl;
  for (int i = 0; i < 43; i++) {
    ya.pop();
  }
  for (int i = 0; i < 46; i++) {
    s.a = i;
    s.b = i+1;
    ya.push_back(s);
  }
  cout<<ya.get_size()<<endl;
  Student res;
  ya.top(res);
  cout<<res.a<<endl;
  ya.at(97, res);
  cout<<res.a<<endl;
}
void yedis_array_test2()
{
  Student s;
  YedisArray<Student, 5> ya;
  for (int i = 1; i <= 12; i++) {
    s.a = i;
    s.b = i+1;
    ya.push_back(s);
  }
  ya.pop();
  ya.pop();
  ya.pop();
  ya.push_back(s);
  ya.push_back(s);
  ya.push_back(s);
  ya.push_back(s);
}
int test_yedis_array_main()
{
  yedis_array_test2();
  return 0;
}
