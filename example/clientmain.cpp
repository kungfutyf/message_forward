#include "client.h"
#include <iostream>
#include <string>

void insert_rank(std::string& str, int start, int end) {
    if (end <= start || end >= str.size()) { return; }
    for (int i = start + 1;i <= end;++i) {
        char tmp = str[i];
        int j = i - 1;
        for (;j >= start;--j) {
            if (str[j] <= tmp) {
                str[j + 1] = tmp;
                break;
            } else {
                str[j + 1] = str[j];
            }
        }
        if (j < start) { str[start] = tmp; }
    }
}

int main() {
    Client client;
    client.run();
    return 0;
}
