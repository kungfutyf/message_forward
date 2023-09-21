#include "recv_server.h"
#include "base.h"
#include <iostream>


const recvdata& RecvBlock::recv_data(int idx) const { 
    if (idx >= _num_blocks) {return NULLBLOCK;}
    return _recv_data[idx];
}
int RecvBlock::find_valid_block(int idx) {
    int cnt = 0;
    while(1) {
        if (idx >= _num_blocks) { idx -= _num_blocks; }
        if (_recv_data[idx].is_occupied == false) {return idx;}
        ++idx;++cnt;
        if (cnt >= _num_blocks) {return -1;}
    }
    return -1;
}
recvdata& RecvBlock::get_valid_block(int idx) { 
    int idxx = find_valid_block(idx);
    if (idxx == -1) {return NULLBLOCK;}
    return _recv_data[idxx];
}

recvdata& RecvBlock::operator [](int idx) {
    if (idx < 0 || idx >= _num_blocks) {return NULLBLOCK;}
    return _recv_data[idx];
}

RecvData::RecvData(int thread_num, int blocknum)
    :_thread_num(thread_num)
    ,_stop(false)
    ,_data(blocknum)
    ,_asyn(0) {
    _client_queue.resize(thread_num);
    _map_client = std::map<node, int, std::function<bool(const node&,const node&)> >
    (std::bind(&RecvData::great, this, std::placeholders::_1,std::placeholders::_2));
    for (int i = 0;i < _thread_num;++i) {
        _map_client.insert(std::make_pair(node(i, 0), 0));
    }
}

int RecvData::init(){
    //udp
    //创建socket
    _sockudp = socket(AF_INET, SOCK_DGRAM, 0);
    //创建地址对象
    struct sockaddr_in recvaddr;
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_port = htons(8000);
    recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //socket绑定地址对象
    if (bind(_sockudp, (struct sockaddr*)&recvaddr, sizeof(recvaddr))) {return -1;}
    //tcp
    //创建socket
    _socktcp = socket(AF_INET, SOCK_STREAM, 0);
    //创建地址
    struct sockaddr_in tcpaddr;
    tcpaddr.sin_family = AF_INET;
    tcpaddr.sin_port = htons(8081);
    tcpaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //socket绑定地址对象
    if (bind(_socktcp, (struct sockaddr*)&tcpaddr, sizeof(tcpaddr))) {return -1;}
    //监听
    if (listen(_socktcp, 10) < 0) {return -1;} 
    std::cout << "sdfsffdasdfs" << std::endl;
    return 0;
}

int RecvData::recv_data() {
    struct sockaddr_in fromaddr{};
    socklen_t slen = sizeof(fromaddr);
    int bufferSize = 8192;
    if (setsockopt(_sockudp, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize)) < 0) {
        std::cerr << "Failed to set receive buffer size." << std::endl;
        return -1;
    }
    int recv_start = 0;
    while(1) {
        recvdata& datab = _data.get_valid_block(recv_start);
        if (!datab.is_valid()) {continue;}
        // tlog("recvdata");
        int byte = recvfrom(_sockudp, datab.data, countn(datab.data), 0, (struct sockaddr*)&fromaddr, &slen);
        // tlog("recvdata success");
        // tlog(byte)
        if (byte < 0) {continue;}
        _queue_nolock.push(&datab, _thread_num);
        // std::cout << "data come" << std::endl;
        {
            _condition.notify_all();
        }
    }
    return 0;
}

void RecvData::recv_and_connect_client() {
    // start_thread_pool();
    _recv_thread = std::thread(&RecvData::recv_data,this);
    while(1) {
        struct sockaddr_in fromaddr;
        socklen_t slen = sizeof(fromaddr);
        int clientfd = accept(_socktcp, (struct sockaddr*)&fromaddr, &slen);
        if (clientfd < 0) { continue; }
        tlog("connected");
        node top = _map_client.begin()->first;
        ++top.count_client;
        {
            _client_queue[top.queue_idx].insert(clientfd);
        }
        _map_client.erase(_map_client.begin());
        _map_client.insert(std::make_pair(top, top.count_client));
        //小根堆，每个元素包含队列索引，以及相应的元素数量，用户增加和断链都需要调整小根堆，每次新增只往堆顶队列中增加数据
        //增加数据后需要向下调整
        need_then_reset();
    }
}

void RecvData::start_thread_pool() {
    std::cout << "threadpool1" << std::endl;
    for (int i = 0;i < _thread_num;++i) {
        _threads_pool.emplace_back(
            [this](int idx){
                while(1) {
                    {
                        std::unique_lock<std::mutex> lock(_mut);
                        // std::cout << "threadpool" << _queue_nolock.len() << std::endl;
                        _condition.wait(lock, [&](){ return ((!_queue_nolock.is_empty()) && _asyn == 0) || _stop;});
                    }
                    ++_asyn;
                    if (_stop) {return;}
                    //处理数据，也就是转发数据
                    auto* front = _queue_nolock.front();
                    if (front && front->count_threads) {
                        // if (front && front->count_threads == 0) { _queue_nolock.pop(); continue;}
                        std::unordered_set<int> clientlis = _client_queue[idx];
                        //设置一个客户端数量上限，比如说5000人，达到这个限制后，清理所有近期连续发送失败次数最多的clientfd，直到降到限制之下
                        //或者连续发送失败次数超过100w次，即连续50秒未连接，直接断掉连接
                        //元素需要有容器索引，发送失败次数，fd，根据发送失败次数排序，升序O(n)
                        //一次发送成功把失败次数清空
                        // tlog("clientlis.size()");
                        // tlog(clientlis.size());
                        for (int clientfd : clientlis) {
                            int bytelen = send(clientfd, front->datablock.load()->data, countn(front->datablock.load()->data),0);
                            // std::cout << "send success:" << bytelen << std::endl;
                        }
                        --front->count_threads;
                        if (front->count_threads <= 0) { 
                            _queue_nolock.pop(); 
                            --_asyn;
                        } else {
                            --_asyn;
                            while(_asyn != 0);
                        }
                    } else {
                        tlog("error");
                        --_asyn;
                        while(_asyn != 0);
                    }
                }
            }, i
        );
    }
}

int RecvData::run() {
    init();
    tlog("init");
    start_thread_pool();//启动转发线程池
    tlog("start_thread_pool");
    recv_and_connect_client();//启动接受线程
    tlog("send_to_client");
    return 0;
}

void RecvData::reset_client() {
    _client_queue.clear();
    _client_queue.resize(_thread_num);
    _map_client.clear();
    for (int i = 0;i < _thread_num;++i) {
        _map_client.insert(std::make_pair(node(i, 0), 0));
    }
    for (std::unordered_set<int>::iterator it = _all_client.begin();it != _all_client.end();++it) {
        node top = _map_client.begin()->first;
        ++top.count_client;
        {
            _client_queue[top.queue_idx].insert(*it);
        }
        _map_client.erase(_map_client.begin());
        _map_client.insert(std::make_pair(top, top.count_client));
    }
}

void RecvData::need_then_reset() {
    node itmini = _map_client.begin()->first;
    node itmax = _map_client.rbegin()->first;
    if (itmax.count_client < itmini.count_client * 2) { return ; }
    reset_client();
}