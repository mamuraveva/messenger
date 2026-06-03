#pragma once

#include "crypto.h"
#include <functional>
#include <string>

class TcpClient {
private:
  int socket = -1;
  Crypto crypto_;

public:
  std::function<void(const std::string &)> on_message_;
  void receive_loop();
  bool create();
  bool connect(const std::string &ip, int port);
  bool do_handshake();
  bool send_message(const std::string &msg);
  bool receive_message();
  bool receive_message_signal();
  bool close();
};
