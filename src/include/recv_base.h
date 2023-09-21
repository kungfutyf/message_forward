#pragma once
#include "base.h"
#include <cstring>
#include <atomic>
#include <iostream>

#define tlog(x) std::cout << x << std::endl;
typedef struct recvdata{
    recvdata(){
        is_occupied = false;
    }
    recvdata(recvdata&& fdata) {
        is_occupied.store(fdata.is_occupied);
        fdata.is_occupied.store(false);
        ::memcpy(data, fdata.data, countn(fdata.data));
    }
    void reset() {
        memset(data, 0, countn(data));
        is_occupied = false;
    }
    bool is_valid() const {
        return !is_occupied;
    }
    char data[100];
    std::atomic<bool> is_occupied;
}rcvd;

typedef struct QNode {
    QNode(int num) {
        datablock = nullptr;
        next = nullptr;
        count_threads = num;
    }
    QNode(recvdata* datablock_, int num) {
        datablock = datablock_;
        datablock.load()->is_occupied = true;
        next = nullptr;
        count_threads = num;
    }
    QNode() {
        datablock = nullptr;
        next = nullptr;
        count_threads = 40;
    }
    ~QNode(){
        next.store(nullptr);
        count_threads = 40;
        recvdata* recvdata = datablock.load();
        if (recvdata != nullptr) {
            recvdata->is_occupied = false;
        }
        datablock.store(nullptr);
    }
    std::atomic<recvdata*> datablock;
    std::atomic<QNode*> next;
    std::atomic<int> count_threads;
    int val;
}qnode;