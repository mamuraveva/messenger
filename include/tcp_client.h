#pragma once

#include <string>
#include "crypto.h"

class TcpClient {
private:
    int socket = -1; 
    Crypto crypto_;

public:
    void receive_loop();
    bool create(); 
    bool connect(const std::string &ip, int port);
    bool do_handshake();
    bool send_message(const std::string &msg);
    bool receive_message();
    bool close();
};
