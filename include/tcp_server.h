#pragma once

#include <mutex>
#include <string>
#include <vector>

class TcpServer {
private:
  int server_socket = -1;
  std::vector<int> clients;
  std::mutex clients_mutex;

public:
  bool start(int port);
  bool validatePort(int port);
  void accept_clients();
  void handle_client(int client_socket);
  void broadcast_message(const std::vector<unsigned char> &data,
                         int sender_socket);
};