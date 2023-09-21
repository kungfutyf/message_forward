#pragma once
#include <string>

class SendData {
public:
    SendData(int numpersecond);
public:
    int run();
private:
    void gen_data();

    int init() ;

    void send_data();

private:
    int _internal;
    int _sendsock;
    std::string _data;
};