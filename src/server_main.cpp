#include "../include/tcp_server.h"
#include <iostream>

int main() {
    TcpServer server;

    if (!server.start(8080)) {
        std::cerr << "Не удалось запустить сервер\n";
        return 1;
    }

    return 0;
}