#include "../include/tcp_server.h"
#include <exception>
#include <iostream>

int main() {
  try {
    TcpServer server;

    if (!server.start(8080)) {
      throw std::runtime_error("Не удалось запустить сервер");
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}