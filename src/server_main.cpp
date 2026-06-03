#include "../include/tcp_server.h"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  try {
    int port = 8080;

    if (argc >= 2) {
      port = std::stoi(argv[1]);
    }

    TcpServer server;

    if (!server.start(port)) {
      throw std::runtime_error("Не удалось запустить сервер");
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}