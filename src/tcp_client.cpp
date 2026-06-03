#include "../include/tcp_client.h"
#include "../include/packet.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sodium.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

void TcpClient::receive_loop() {
  while (true) {
    try {
      if (!receive_message()) {
        break;
      }
    } catch (const std::exception &e) {
      std::cout << e.what() << std::endl;
      break;
    }
  }
}

bool TcpClient::create() {
  try {
    socket = ::socket(AF_INET, SOCK_STREAM, 0);

    if (socket < 0) {
      throw std::runtime_error("Ошибка socket: " + std::string(strerror(errno)));
    }

    return true;
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
}

bool TcpClient::connect(const std::string &ip, int port) {
  try {
    if (socket < 0) {
      throw std::runtime_error("Ошибка: сокет не создан");
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
      throw std::runtime_error("Ошибка: некорректный IP-адрес");
    }

    if (::connect(socket, reinterpret_cast<sockaddr *>(&serverAddr),
                  sizeof(serverAddr)) < 0) {
      throw std::runtime_error("Ошибка connect: " + std::string(strerror(errno)));
    }

    return true;
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
}

bool TcpClient::do_handshake() {
  try {
    std::vector<unsigned char> role_packet;

    if (!recv_packet(socket, role_packet) || role_packet.empty()) {
      throw std::runtime_error("Ошибка получения роли");
    }

    unsigned char role = role_packet[0];

    if (role == 0x01) {
      std::cout << "Роль: dialer" << std::endl;

      std::vector<unsigned char> my_pub(
          crypto_.pub_key(), crypto_.pub_key() + crypto_kx_PUBLICKEYBYTES);

      if (!send_packet(socket, my_pub)) {
        throw std::runtime_error("Ошибка отправки pub_key");
      }

      std::vector<unsigned char> their_pub;

      if (!recv_packet(socket, their_pub) ||
          their_pub.size() != crypto_kx_PUBLICKEYBYTES) {
        throw std::runtime_error("Ошибка получения pub_key");
      }

      crypto_.setup_as_dialer(their_pub.data());
    } else {
      std::cout << "Роль: receiver" << std::endl;

      std::vector<unsigned char> their_pub;

      if (!recv_packet(socket, their_pub) ||
          their_pub.size() != crypto_kx_PUBLICKEYBYTES) {
        throw std::runtime_error("Ошибка получения pub_key");
      }

      std::vector<unsigned char> my_pub(
          crypto_.pub_key(), crypto_.pub_key() + crypto_kx_PUBLICKEYBYTES);

      if (!send_packet(socket, my_pub)) {
        throw std::runtime_error("Ошибка отправки pub_key");
      }

      crypto_.setup_as_receiver(their_pub.data());
    }

    std::cout << "Соединение зашифровано" << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
}

bool TcpClient::send_message(const std::string &msg) {
  try {
    if (socket < 0) {
      throw std::runtime_error("Ошибка: сокет не создан");
    }

    std::vector<unsigned char> data = crypto_.encrypt(msg);

    if (!send_packet(socket, data)) {
      throw std::runtime_error("Ошибка отправки пакета");
    }

    return true;
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
}

bool TcpClient::receive_message() {
  try {
    if (socket < 0) {
      throw std::runtime_error("Ошибка: сокет не создан");
    }

    std::vector<unsigned char> data;

    if (!recv_packet(socket, data)) {
      throw std::runtime_error("Сервер закрыл соединение");
    }

    std::string msg = crypto_.decrypt(data);

    if (on_message_) {
      on_message_(msg);
    } else {
      std::cout << msg << std::endl;
    }

    return true;
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
}

bool TcpClient::close() {
  try {
    if (socket < 0) {
      return true;
    }

    ::close(socket);
    socket = -1;

    std::cout << "Соединение закрыто" << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
}