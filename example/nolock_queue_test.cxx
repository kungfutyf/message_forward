#include "nolock_queue.h"
#include <thread>
#include <iostream>
#include <mutex>

std::atomic<int> flag(0);
std::atomic<bool> asyn(false);
std::string vvv;
std::mutex mut;


typedef struct Node {
    Node(int num) {
        val = num;
        next = nullptr;
    }

    Node() {
        val = 0;
        next = nullptr;
    }

    std::atomic<int> val;
    std::atomic<Node*> next;
}nd;

std::ostream& operator << (std::ostream& out, QueueNoBlock<Node, int>&qnb) {
    Node* front = qnb.front();
    while(front) {
        out << front->val << ',';
        front = front->next.load();
    }
    return out;
}
#define NUM 40
#define POPNUM 14
#define PUSHNUM 15
std::atomic<int> ff(POPNUM);

void push(QueueNoBlock<Node, int>& qnb, int val) {
    while(!asyn);
    int c = NUM;
    while (c--)
    qnb.push(val);
    ++flag;
}

int pop(QueueNoBlock<Node, int>& qnb) {
    std::unique_lock<std::mutex> lock(mut, std::defer_lock);
    while (!qnb.is_empty()) {
        if (!qnb.is_empty()) qnb.pop();
        else {continue;}
    }
    --ff;
    return 0;
}

int hah() {
    return 0;
}

int main() {
    // QueueNoBlock<Node, int> qnb;
    // for (int i = 0;i < PUSHNUM;++i) {
    //     std::thread th(push, std::ref(qnb), i+1);
    //     th.detach();
    // }
    // asyn = true;
    // while(flag != PUSHNUM) {}
    // // std::cout << qnb << std::endl;
    // // std::cout << std::endl;
    // for (int i = 0;i < POPNUM;++i) {
    //     std::thread th(pop, std::ref(qnb));
    //     th.detach();
    // }
    // while(ff > 0) {
        
    // }
    int a[1] = {2};
    a[0] = 2;
    a[1] = 3;
    for (int i = 0;i < 100;++i) {
        a[i] = i;
    }
    hah();
    *((int*)0x00000) = 5;
    return 0;
}