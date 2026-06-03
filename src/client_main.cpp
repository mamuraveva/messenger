#include "../include/tcp_client.h"
#include <iostream>
#include <string>
#include <thread>

int main() {
  TcpClient client;
  if (!client.create()) {
    return 1;
  }
  if (!client.connect("127.0.0.1", 8080)) {
    return 1;
  }
  if (!client.do_handshake()) {
    return 1;
  }
  std::thread receive_thread(
      &TcpClient::receive_loop,
      &client); // чтобы получить адрес кода для вызова и чтобы поток работал с
                // оригиналом, а не копией
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
      break;
    }
  }
  client.close();
  receive_thread.join();
  return 0;
}