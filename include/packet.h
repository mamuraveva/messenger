#pragma once

#include <vector>

bool send_packet(int socket, const std::vector<unsigned char>& data);
bool recv_packet(int socket, std::vector<unsigned char>& data);

