#include "base.h"
#include "nolock_queue.h"
#include "send_server.h"
#include "recv_base.h"
#include <time.h>
#include <stdlib.h>
#include <thread>

SendData::SendData(int numpersecond) {
    _internal = 1000000 / numpersecond;
    _data.resize(100);
}

void SendData::gen_data() {
    srand(time(nullptr));
    // #pragma omp parallel for num_threads(20)
    for (int i = 0;i < _data.size();++i) {
        _data[i] = (rand()%10) + '0';
    }
}

int SendData::init() {
    //创建套接字
    _sendsock = socket(AF_INET, SOCK_DGRAM, 0);
    return 0;
}

void SendData::send_data() {
    struct sockaddr_in toaddr;
    toaddr.sin_family = AF_INET;
    toaddr.sin_port = htons(8000);
    toaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    while(1) {
        gen_data();
        int byte = sendto(_sendsock, _data.data(), _data.size(), 0, (struct sockaddr*)&toaddr, sizeof(toaddr));
        if (byte < 0) {
            continue;
        }
        tlog("send success");
        std::this_thread::sleep_for(std::chrono::microseconds(_internal));
    }
}

int SendData::run() {
    init();
    send_data();
    return 0;
}
