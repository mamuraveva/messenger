#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "tcp_client.h"
#include "tcp_server.h"

TEST_CASE("Testing TcpClient") {
    TcpClient client;

    SUBCASE("Success: Create socket") {
        CHECK(client.create() == true);
    }

    SUBCASE("Fail: Connect to non-existent server") {
        client.create();
        CHECK(client.connect("127.0.0.1", 9999) == false);
    }

    SUBCASE("Fail: Send message without connection") {
        client.create();
        CHECK(client.send_message("test") == false);
    }
}

TEST_CASE("Testing TcpServer") {
    TcpServer server;

    SUBCASE("Success: Valid port") {
        CHECK(server.validatePort(8080) == true);
        CHECK(server.validatePort(65535) == true);
    }

    SUBCASE("Fail: Invalid port values") {
        CHECK(server.validatePort(-1) == false);
        CHECK(server.validatePort(0) == false);
        CHECK(server.validatePort(70000) == false);
    }
    
    SUBCASE("Fail: Start server on privileged port") {
        // Порты ниже 1024 требуют прав root, 
        // поэтому start должен вернуть false, если запускаем от пользователя
        CHECK(server.start(80) == false);
    }
}