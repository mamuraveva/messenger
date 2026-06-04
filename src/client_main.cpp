#include "../include/tcp_client.h"
#include <exception>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
  try {
    std::string host = "127.0.0.1";
    int port = 8080;

    if (argc >= 2) {
      host = argv[1];
    }

    if (argc >= 3) {
      port = std::stoi(argv[2]);
    }

    TcpClient client;

    if (!client.create()) {
      throw std::runtime_error("Не удалось создать сокет клиента");
    }

    if (!client.connect(host, port)) {
      throw std::runtime_error("Не удалось подключиться к серверу");
    }

    if (!client.do_handshake()) {
      throw std::runtime_error("Не удалось выполнить обмен ключами");
    }

    std::thread receive_thread(&TcpClient::receive_loop, &client);

    std::string username;
    std::cout << "Введите имя: ";
    std::getline(std::cin, username);

    std::string msg;

    while (true) {
      std::getline(std::cin, msg);

      if (msg == "/exit") {
        break;
      }

      if (!client.send_message(username + ": " + msg)) {
        throw std::runtime_error("Не удалось отправить сообщение");
      }
    }

    client.close();

    if (receive_thread.joinable()) {
      receive_thread.join();
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}