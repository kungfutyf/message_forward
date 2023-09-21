#include "send_server.h"

int main() {
    SendData send(200);
    send.run();
    return 0;
}