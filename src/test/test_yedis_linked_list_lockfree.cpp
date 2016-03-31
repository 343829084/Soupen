#include "../ds/yedis_linked_list_lockfree.h"
#include<iostream>
#include<thread>
using namespace std;
using namespace yedis_datastructures;
YedisLinkedListLF<LinkedListNode> *p = new YedisLinkedListLF<LinkedListNode>();
void ins0()
{
  for (int i = 0; i <= 1000000; i+=2) {
    LinkedListNode *ln = new LinkedListNode(i, nullptr);
    p->insert(ln);
  }
}
void ins1()
{
  for (int i = 1; i <= 10000; i+=2) {
    LinkedListNode *ln = new LinkedListNode(i, nullptr);
    p->insert(ln);
  }
}
int u()
{
  thread t1(ins0);
  thread t2(ins1);
  t1.join();
  t2.join();
  p->show();
  return 0;
}
