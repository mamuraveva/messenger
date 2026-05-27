#include "../include/tcp_server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include "../include/packet.h"
#include <chrono>

bool TcpServer::validatePort(int port) {
    return port >= 1024 && port <= 65535;
}

bool TcpServer::start(int port) {
    if (!validatePort(port)) {
        std::cerr << "Некорректный порт\n";
        return false;
    }
    server_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Ошибка socket: " << strerror(errno) << std::endl;
        return false;
    }
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (::bind(server_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        std::cerr << "Ошибка bind: " << strerror(errno) << std::endl;
        ::close(server_socket);
        server_socket = -1;
        return false;
    }
    if (::listen(server_socket, 5) < 0) {
        std::cerr << "Ошибка listen: " << strerror(errno) << std::endl;
        ::close(server_socket);
        server_socket = -1;
        return false;
    }
    std::cout << "Сервер запущен на порту " << port << std::endl;
    std::cout << "Ожидание клиентов...\n";
    accept_clients();
    return true;
}

void TcpServer::accept_clients() {
    int client_a = ::accept(server_socket, nullptr, nullptr);
    if (client_a < 0) {
        std::cerr << "Ошибка accept: " << strerror(errno) << std::endl;
        return;
    }
    std::cout << "Первый клиент подключился: socket = " << client_a << std::endl;
    int client_b = ::accept(server_socket, nullptr, nullptr);
    if (client_b < 0) {
        std::cerr << "Ошибка accept: " << strerror(errno) << std::endl;
        return;
    }
    std::cout << "Второй клиент подключился: socket = " << client_b << std::endl;
    send_packet(client_a, {0x01}); // dialer
    send_packet(client_b, {0x02}); // receiver
    std::vector<unsigned char> pub_a;
    if (!recv_packet(client_a, pub_a)) {
        std::cerr << "Ошибка получения pub_key A\n";
        return;
    }
    send_packet(client_b, pub_a);
    std::vector<unsigned char> pub_b;
    if (!recv_packet(client_b, pub_b)) {
        std::cerr << "Ошибка получения pub_key B\n";
        return;
    }
    send_packet(client_a, pub_b);
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_a);
        clients.push_back(client_b);
    }
    std::thread(&TcpServer::handle_client, this, client_a).detach();
    std::thread(&TcpServer::handle_client, this, client_b).detach();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void TcpServer::handle_client(int client_socket) {
    while (true) {
        std::vector<unsigned char> data;
        if (!recv_packet(client_socket, data)) {
            std::cout << "Клиент отключился: socket = " << client_socket << std::endl;
            break;
        }
        std::cout << "Получен пакет от socket " << client_socket << ", размер: " << data.size() << " байт" << std::endl;
        broadcast_message(data, client_socket);
    }
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    }
    ::close(client_socket);
}

void TcpServer::broadcast_message(const std::vector<unsigned char>& data, int sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i] == sender_socket) {
            continue;
        }
        if (!send_packet(clients[i], data)) {
            std::cerr << "Ошибка отправки пакета клиенту socket " << clients[i] << std::endl;
        }
    }
}