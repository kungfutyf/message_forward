#include "client.h"
#include "base.h"
#include <iostream>
#include <chrono>

Client::Client() {
    recv_buff.resize(100);
    init();
}
int Client::init() {
    _ipaddr = SERVER_IP;
    //创建socket
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    //创建地址对象
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);
    serverAddr.sin_addr.s_addr = inet_addr(_ipaddr.c_str());
    //连接
    if (connect(_sock,(struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {return -1;}
    //
    return 0;
}
int Client::start() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    int cnt = 0;
    //接受数据
    while(1) {
        int byte = recv(_sock, (void*)recv_buff.data(), recv_buff.size(), 0);
        if (byte <= 0) {continue;}
        if (byte < recv_buff.size()) { recv_buff[byte - 1] = '0';}
        // std::cout << recv_buff << std::endl; 
        ++cnt;
        if (cnt == 200) {
            auto noww = std::chrono::system_clock::now();
            auto mss = std::chrono::duration_cast<std::chrono::milliseconds>(noww.time_since_epoch()).count();
            std::cout << mss - ms << std::endl;
        }
    }
}

int Client::run() {
    init();
    start();
    return 0;
}