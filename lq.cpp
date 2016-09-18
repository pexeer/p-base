#include "p/base/queue.h"
#include <iostream>
#include <thread>

constexpr int kFlag = 10000;

struct Node {
  Node *next;
  int thread_id = -1;
  int value = -1;
};

void push(p::base::LinkedQueue<Node> &lq, int thread_id) {
  int id = 0;
  for (int i = 0; i < 10000; ++i) {
    Node *p = new Node[kFlag];
    for (int j = 1; j < kFlag; ++j) {
      Node &tmp = p[j];
      tmp.thread_id = thread_id;
      tmp.value = ++id;
      lq.push_back(&tmp);
    }
    Node &tmp = p[0];
    p[0].thread_id = thread_id;
    p[0].value = 0;
    lq.push_back(&tmp);
  }
}

void pop(p::base::LinkedQueue<Node> &lq) {
  while (1) {
    Node *ret = lq.pop_front();
    if (ret) {
      std::cout << ret->thread_id << "\t" << ret->value << "\n";
      if (ret->value == 0) {
        delete[] ret;
        std::cout << std::flush;
      }
    }
  }
}

int main() {
  p::base::LinkedQueue<Node> lq;
  for (int i = 0; i < 16; ++i) {
    std::thread tmp(push, std::ref(lq), i);
    tmp.detach();
  }
  std::thread tmp(pop, std::ref(lq));
  tmp.join();
  return 0;
}
