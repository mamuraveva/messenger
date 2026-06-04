#include "../include/packet.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sys/socket.h>

static bool send_all(int socket, const unsigned char *data, size_t size) {
  size_t total_sent = 0;
  while (total_sent < size) {
    ssize_t bytes_sent =
        ::send(socket, data + total_sent, size - total_sent, 0);
    if (bytes_sent <= 0) {
      return false;
    }
    total_sent += bytes_sent;
  }
  return true;
}

static bool recv_all(int socket, unsigned char *data, size_t size) {
  size_t total_received = 0;
  while (total_received < size) {
    ssize_t bytes_received =
        ::recv(socket, data + total_received, size - total_received, 0);
    if (bytes_received <= 0) {
      return false;
    }
    total_received += bytes_received;
  }
  return true;
}

bool send_packet(int socket, const std::vector<unsigned char> &data) {
  uint32_t size = static_cast<uint32_t>(data.size());
  uint32_t network_size = htonl(size);
  if (!send_all(socket, reinterpret_cast<unsigned char *>(&network_size),
                sizeof(network_size))) {
    return false;
  }
  if (!data.empty()) {
    if (!send_all(socket, data.data(), data.size())) {
      return false;
    }
  }
  return true;
}

bool recv_packet(int socket, std::vector<unsigned char> &data) {
  uint32_t network_size = 0;
  if (!recv_all(socket, reinterpret_cast<unsigned char *>(&network_size),
                sizeof(network_size))) {
    return false;
  }
  uint32_t size = ntohl(network_size);
  data.resize(size);
  if (size > 0) {
    if (!recv_all(socket, data.data(), size)) {
      return false;
    }
  }
  return true;
}