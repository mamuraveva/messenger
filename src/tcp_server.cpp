#include "../include/tcp_server.h"
#include "../include/packet.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

static std::string sock_error(const std::string &context) {
  return context + ": " + std::string(strerror(errno));
}

bool TcpServer::validatePort(int port) {
  return port >= 1024 && port <= 65535;
}

bool TcpServer::start(int port) {
  if (!validatePort(port)) {
    throw std::invalid_argument("Некорректный порт: " + std::to_string(port) +
                                ". Допустимый диапазон: 1024–65535");
  }

  server_socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    throw std::runtime_error(sock_error("Ошибка socket"));
  }

  sockaddr_in serverAddr;
  std::memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (::bind(server_socket, reinterpret_cast<sockaddr *>(&serverAddr),
             sizeof(serverAddr)) < 0) {
    ::close(server_socket);
    server_socket = -1;
    throw std::runtime_error(sock_error("Ошибка bind"));
  }

  if (::listen(server_socket, 5) < 0) {
    ::close(server_socket);
    server_socket = -1;
    throw std::runtime_error(sock_error("Ошибка listen"));
  }

  std::cout << "Сервер запущен на порту " << port << std::endl;
  std::cout << "Ожидание клиентов...\n";

  try {
    accept_clients();
  } catch (const std::exception &e) {
    std::cerr << "Сервер завершил работу с ошибкой: " << e.what() << std::endl;

    if (server_socket >= 0) {
      ::close(server_socket);
      server_socket = -1;
    }

    return false;
  }

  return true;
}

void TcpServer::accept_clients() {
  while (true) {
    int client_a = ::accept(server_socket, nullptr, nullptr);
    if (client_a < 0) {
      throw std::runtime_error(sock_error("Ошибка accept (первый клиент пары)"));
    }

    std::cout << "Первый клиент пары подключился: socket = "
              << client_a << std::endl;

    int client_b = ::accept(server_socket, nullptr, nullptr);
    if (client_b < 0) {
      ::close(client_a);
      throw std::runtime_error(sock_error("Ошибка accept (второй клиент пары)"));
    }

    std::cout << "Второй клиент пары подключился: socket = "
              << client_b << std::endl;

    if (!send_packet(client_a, {0x01})) {
      throw std::runtime_error("Ошибка отправки роли клиенту A (dialer)");
    }

    if (!send_packet(client_b, {0x02})) {
      throw std::runtime_error("Ошибка отправки роли клиенту B (receiver)");
    }

    std::vector<unsigned char> pub_a;
    if (!recv_packet(client_a, pub_a)) {
      throw std::runtime_error("Ошибка получения pub_key от клиента A");
    }

    if (!send_packet(client_b, pub_a)) {
      throw std::runtime_error("Ошибка пересылки pub_key A клиенту B");
    }

    std::vector<unsigned char> pub_b;
    if (!recv_packet(client_b, pub_b)) {
      throw std::runtime_error("Ошибка получения pub_key от клиента B");
    }

    if (!send_packet(client_a, pub_b)) {
      throw std::runtime_error("Ошибка пересылки pub_key B клиенту A");
    }

    {
      std::lock_guard<std::mutex> lock(clients_mutex);
      pairs.push_back({client_a, client_b});
    }

    std::cout << "Создана новая пара: "
              << client_a << " <-> " << client_b << std::endl;

    std::thread(&TcpServer::handle_client, this, client_a).detach();
    std::thread(&TcpServer::handle_client, this, client_b).detach();
  }
}

void TcpServer::handle_client(int client_socket) {
  try {
    while (true) {
      std::vector<unsigned char> data;

      if (!recv_packet(client_socket, data)) {
        std::cout << "Клиент отключился: socket = "
                  << client_socket << std::endl;
        break;
      }

      std::cout << "Получен пакет от socket " << client_socket
                << ", размер: " << data.size() << " байт" << std::endl;

      broadcast_message(data, client_socket);
    }
  } catch (const std::exception &e) {
    std::cerr << "Ошибка при обработке клиента socket "
              << client_socket << ": " << e.what() << std::endl;
  }

  {
    std::lock_guard<std::mutex> lock(clients_mutex);

    pairs.erase(
        std::remove_if(
            pairs.begin(),
            pairs.end(),
            [client_socket](const std::pair<int, int> &pair) {
              return pair.first == client_socket ||
                     pair.second == client_socket;
            }),
        pairs.end());
  }

  ::close(client_socket);
}

void TcpServer::broadcast_message(const std::vector<unsigned char> &data,
                                  int sender_socket) {
  std::lock_guard<std::mutex> lock(clients_mutex);

  for (const auto &pair : pairs) {
    int client_a = pair.first;
    int client_b = pair.second;

    try {
      if (sender_socket == client_a) {
        if (!send_packet(client_b, data)) {
          throw std::runtime_error("Ошибка отправки клиенту " +
                                   std::to_string(client_b));
        }
        return;
      }

      if (sender_socket == client_b) {
        if (!send_packet(client_a, data)) {
          throw std::runtime_error("Ошибка отправки клиенту " +
                                   std::to_string(client_a));
        }
        return;
      }
    } catch (const std::exception &e) {
      std::cerr << "Ошибка отправки пакета: "
                << e.what() << std::endl;
    }
  }
}