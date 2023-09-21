#include "recv_server.h"


int main() {
    RecvData recv(2);
    recv.run();
    return 0;
}