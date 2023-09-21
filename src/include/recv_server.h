#pragma once

#include "nolock_queue.h"
#include "recv_base.h"
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
// #include <queue>
#include <functional>
#include <map>

class RecvData;
class RecvBlock {
public:
    static recvdata NULLBLOCK;

public:
    RecvBlock(int num):_num_blocks(num) {
        _recv_data.resize(num);
    }
    // RecvBlock(){
    //     _recv_data.resize(_num_blocks);
    // }
public:
    const recvdata& recv_data(int idx) const;
    int find_valid_block(int idx);
    recvdata& get_valid_block(int idx);

    recvdata& operator [](int idx) ;

private:
    std::vector<recvdata> _recv_data;
    int _num_blocks;
};

recvdata RecvBlock::NULLBLOCK;

class RecvData {
public:
    RecvData(int thread_num, int blocknum = 20000);
    ~RecvData(){ _stop = true; }

public:
    int run();

private:
    int init();

    void recv_and_connect_client();

    void start_thread_pool();

    int recv_data();

private:
    void reset_client();

    void need_then_reset();

private:
    struct node {
        node() {
            queue_idx = -1;
            count_client = 0;
        }
        node(int queueidx, int countclient) {
            queue_idx = queueidx;
            count_client = countclient;
        }
        void clear() {
            queue_idx = -1;
            count_client = 0;
        }
        int queue_idx;
        int count_client;
    };
    bool great(const node& a, const node& b) { return a.count_client > b.count_client; }
private:
    int _sockudp;
    int _socktcp;
    int _thread_num;
    std::atomic<bool> _stop;
    std::atomic<int> _asyn;
    std::mutex _mut;
    std::mutex _mut_sync;
    std::condition_variable _condition;
    std::condition_variable _condition_sync;
    std::vector<std::thread> _threads_pool;
    std::thread _recv_thread;
    RecvBlock _data;
    std::map<node, int, std::function<bool(const node&, const node&)> > _map_client;
    QueueNoBlock<QNode, recvdata*, int> _queue_nolock;
    std::unordered_set<int> _all_client;
    std::vector<std::unordered_set<int> > _client_queue;
    // std::priority_queue<node, std::vector<node>, std::function<bool(const node&,const node&)> > _count_client;
    // std::priority_queue<node, std::vector<node>, std::function<bool(const node&, const node&)> > _count_client;
};