#pragma once

#include <string>

#define SERVER_IP "127.0.0.1"
class Client {
public:
    Client();
    ~Client(){}
public:
    int run();

private:
    int init();
    int start();
private:
    int _sock;
    std::string _ipaddr;
    std::string recv_buff;
};