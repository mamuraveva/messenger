#include "../include/tcp_client.h"
#include "../include/packet.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sodium.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

void TcpClient::receive_loop() {
  while (true) {
    if (!receive_message()) {
      break;
    }
  }
}

bool TcpClient::create() {
  socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    std::cerr << "Ошибка socket: " << strerror(errno) << std::endl;
    return false;
  }
  return true;
}

bool TcpClient::connect(const std::string &ip, int port) {
  if (socket < 0) {
    std::cerr << "Ошибка: сокет не создан\n";
    return false;
  }
  sockaddr_in serverAddr;
  std::memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "Ошибка: некорректный ip-адрес\n";
    return false;
  }
  if (::connect(socket, reinterpret_cast<sockaddr *>(&serverAddr),
                sizeof(serverAddr)) < 0) {
    std::cerr << "Ошибка connect: " << strerror(errno) << std::endl;
    return false;
  }
  return true;
}

bool TcpClient::do_handshake() {
  std::vector<unsigned char> role_packet;
  if (!recv_packet(socket, role_packet) || role_packet.empty()) {
    std::cerr << "Ошибка получения роли\n";
    return false;
  }
  unsigned char role = role_packet[0];
  if (role == 0x01) {
    std::cout << "Роль: dialer\n";
    std::vector<unsigned char> my_pub(
        crypto_.pub_key(), crypto_.pub_key() + crypto_kx_PUBLICKEYBYTES);
    if (!send_packet(socket, my_pub)) {
      std::cerr << "Ошибка отправки pub_key\n";
      return false;
    }
    std::vector<unsigned char> their_pub;
    if (!recv_packet(socket, their_pub) ||
        their_pub.size() != crypto_kx_PUBLICKEYBYTES) {
      std::cerr << "Ошибка получения pub_key\n";
      return false;
    }
    crypto_.setup_as_dialer(their_pub.data());
  } else {
    std::cout << "Роль: receiver\n";
    std::vector<unsigned char> their_pub;
    if (!recv_packet(socket, their_pub) ||
        their_pub.size() != crypto_kx_PUBLICKEYBYTES) {
      std::cerr << "Ошибка получения pub_key\n";
      return false;
    }
    std::vector<unsigned char> my_pub(
        crypto_.pub_key(), crypto_.pub_key() + crypto_kx_PUBLICKEYBYTES);
    if (!send_packet(socket, my_pub)) {
      std::cerr << "Ошибка отправки pub_key\n";
      return false;
    }
    crypto_.setup_as_receiver(their_pub.data());
  }
  std::cout << "Соединение зашифровано\n";
  return true;
}

bool TcpClient::send_message(const std::string &msg) {
  if (socket < 0) {
    std::cerr << "Ошибка: сокет не создан\n";
    return false;
  }
  std::vector<unsigned char> data = crypto_.encrypt(msg);
  if (!send_packet(socket, data)) {
    std::cerr << "Ошибка отправки пакета\n";
    return false;
  }
  return true;
}

bool TcpClient::receive_message() {
  if (socket < 0) {
    std::cerr << "Ошибка: сокет не создан\n";
    return false;
  }
  std::vector<unsigned char> data;
  if (!recv_packet(socket, data)) {
    std::cout << "Сервер закрыл соединение\n";
    return false;
  }
  std::string msg = crypto_.decrypt(data);
  std::cout << msg << std::endl;
  return true;
}

bool TcpClient::close() {
  if (socket < 0) {
    std::cerr << "Ошибка: сокет не создан\n";
    return false;
  }
  ::close(socket);
  socket = -1;
  std::cout << "Соединение закрыто\n";
  return true;
}
