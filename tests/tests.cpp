#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/crypto.h"
#include "../include/packet.h"
#include "../include/tcp_server.h"
#include "doctest.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
static std::pair<int, int> make_socket_pair() {
  int fds[2];
  REQUIRE(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
  return {fds[0], fds[1]};
}

struct SockPair {
  int a, b;
  SockPair() {
    auto [x, y] = make_socket_pair();
    a = x;
    b = y;
  }
  ~SockPair() {
    ::close(a);
    ::close(b);
  }
};

//  1. TcpServer::validatePort
TEST_CASE("validatePort — граничные и типичные значения") {
  TcpServer srv;

  SUBCASE("минимально допустимый порт") {
    CHECK(srv.validatePort(1024) == true);
  }
  SUBCASE("максимально допустимый порт") {
    CHECK(srv.validatePort(65535) == true);
  }
  SUBCASE("порт приложения") { CHECK(srv.validatePort(8080) == true); }
  SUBCASE("популярный порт 12345") { CHECK(srv.validatePort(12345) == true); }
  SUBCASE("недопустимый порт 0") { CHECK(srv.validatePort(0) == false); }
  SUBCASE("недопустимый зарезервированный порт 1") {
    CHECK(srv.validatePort(1) == false);
  }
  SUBCASE("недопустимый порт 1023") { CHECK(srv.validatePort(1023) == false); }
  SUBCASE("недопустимый порт 65536") {
    CHECK(srv.validatePort(65536) == false);
  }
  SUBCASE("недопустимый отрицательный порт") {
    CHECK(srv.validatePort(-1) == false);
  }
}

// 2. Crypto — генерация ключей
TEST_CASE("Crypto — генерация пар ключей") {
  SUBCASE("два объекта создают разные публичные ключи") {
    Crypto a, b;
    int diff = std::memcmp(a.pub_key(), b.pub_key(), crypto_kx_PUBLICKEYBYTES);
    CHECK(diff != 0);
  }
  SUBCASE("pub_key ненулевой") {
    Crypto c;
    std::vector<unsigned char> zero(crypto_kx_PUBLICKEYBYTES, 0);
    int diff = std::memcmp(c.pub_key(), zero.data(), crypto_kx_PUBLICKEYBYTES);
    CHECK(diff != 0);
  }
  SUBCASE("повторное создание объекта не падает") {
    CHECK_NOTHROW([] {
      Crypto c1;
      Crypto c2;
      Crypto c3;
    }());
  }
}

//  3. Crypto — handshake
TEST_CASE("Crypto — handshake dialer/receiver") {
  SUBCASE("корректный handshake не бросает исключений") {
    Crypto dialer, receiver;
    CHECK_NOTHROW(dialer.setup_as_dialer(receiver.pub_key()));
    CHECK_NOTHROW(receiver.setup_as_receiver(dialer.pub_key()));
  }
  SUBCASE(
      "после handshake стороны договорились об одном ключе (encrypt→decrypt)") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());
    std::string msg = "handshake check";
    auto cipher = dialer.encrypt(msg);
    std::string plain = receiver.decrypt(cipher);
    CHECK(plain == msg);
  }
  SUBCASE("setup_as_dialer с мусорным ключом бросает исключение") {
    Crypto dialer;
    unsigned char garbage[crypto_kx_PUBLICKEYBYTES] = {};
    CHECK_THROWS(dialer.setup_as_dialer(garbage));
  }
  SUBCASE("setup_as_receiver с мусорным ключом бросает исключение") {
    Crypto receiver;
    unsigned char garbage[crypto_kx_PUBLICKEYBYTES] = {};
    CHECK_THROWS(receiver.setup_as_receiver(garbage));
  }
}

//  4. Crypto — encrypt
TEST_CASE("Crypto — encrypt") {
  SUBCASE("encrypt до handshake бросает исключение") {
    Crypto c;
    CHECK_THROWS(c.encrypt("hello"));
  }
  SUBCASE("encrypt возвращает данные длиннее исходного текста (nonce + MAC)") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());
    std::string msg = "test";
    auto cipher = dialer.encrypt(msg);
    CHECK(cipher.size() > msg.size());
  }
  SUBCASE("encrypt пустой строки не падает") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());
    CHECK_NOTHROW(dialer.encrypt(""));
  }
  SUBCASE("два вызова encrypt одного текста дают разный шифртекст (случайный "
          "nonce)") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());
    auto c1 = dialer.encrypt("same");
    auto c2 = dialer.encrypt("same");
    CHECK(c1 != c2);
  }
  SUBCASE("encrypt длинного сообщения (1 МБ) не падает") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());
    std::string big(1024 * 1024, 'A');
    CHECK_NOTHROW(dialer.encrypt(big));
  }
}

//  5. Crypto — decrypt
TEST_CASE("Crypto — decrypt") {
  SUBCASE("decrypt до handshake бросает исключение") {
    Crypto c;
    std::vector<unsigned char> junk(64, 0xFF);
    CHECK_THROWS(c.decrypt(junk));
  }
  SUBCASE("decrypt слишком короткого пакета бросает исключение") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());

    std::vector<unsigned char> tiny(3, 0x00);
    CHECK_THROWS(receiver.decrypt(tiny));
  }
  SUBCASE("decrypt повреждённого шифртекста бросает исключение") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());

    auto cipher = dialer.encrypt("secret");
    cipher.back() ^= 0xFF; // портим последний байт
    CHECK_THROWS(receiver.decrypt(cipher));
  }
  SUBCASE("decrypt шифртекста чужого отправителя бросает исключение") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());

    Crypto impostor, receiver2;
    impostor.setup_as_dialer(receiver2.pub_key());
    receiver2.setup_as_receiver(impostor.pub_key());

    auto bad_cipher = impostor.encrypt("evil");
    CHECK_THROWS(receiver.decrypt(bad_cipher)); // receiver ждёт ключ dialer-а
  }
  SUBCASE("decrypt пустого зашифрованного сообщения возвращает пустую строку") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());

    auto cipher = dialer.encrypt("");
    std::string plain = receiver.decrypt(cipher);
    CHECK(plain.empty());
  }
  SUBCASE("round-trip: encrypt→decrypt сохраняет данные") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());

    std::string original = "Привет, мир! Hello, world! 12345 !@#$%";
    auto cipher = dialer.encrypt(original);
    CHECK(receiver.decrypt(cipher) == original);
  }
  SUBCASE("round-trip длинного сообщения (1 МБ)") {
    Crypto dialer, receiver;
    dialer.setup_as_dialer(receiver.pub_key());
    receiver.setup_as_receiver(dialer.pub_key());

    std::string big(1024 * 1024, 'Z');
    auto cipher = dialer.encrypt(big);
    CHECK(receiver.decrypt(cipher) == big);
  }
}

//  6. packet — send_packet / recv_packet
TEST_CASE("send_packet / recv_packet — базовый обмен") {
  SUBCASE("отправка и получение непустого пакета") {
    SockPair sp;
    std::vector<unsigned char> payload = {0x01, 0x02, 0x03, 0xAB, 0xFF};
    REQUIRE(send_packet(sp.a, payload));

    std::vector<unsigned char> received;
    REQUIRE(recv_packet(sp.b, received));
    CHECK(received == payload);
  }
  SUBCASE("отправка пустого пакета") {
    SockPair sp;
    std::vector<unsigned char> empty;
    REQUIRE(send_packet(sp.a, empty));

    std::vector<unsigned char> received;
    REQUIRE(recv_packet(sp.b, received));
    CHECK(received.empty());
  }
  SUBCASE("несколько пакетов подряд не смешиваются") {
    SockPair sp;
    std::vector<unsigned char> p1 = {0x11, 0x22};
    std::vector<unsigned char> p2 = {0xAA, 0xBB, 0xCC};
    REQUIRE(send_packet(sp.a, p1));
    REQUIRE(send_packet(sp.a, p2));

    std::vector<unsigned char> r1, r2;
    REQUIRE(recv_packet(sp.b, r1));
    REQUIRE(recv_packet(sp.b, r2));
    CHECK(r1 == p1);
    CHECK(r2 == p2);
  }
  SUBCASE("большой пакет (64 КБ)") {
    SockPair sp;
    std::vector<unsigned char> big(64 * 1024, 0x5A);
    std::vector<unsigned char> received;
    std::thread sender([&] { REQUIRE(send_packet(sp.a, big)); });
    REQUIRE(recv_packet(sp.b, received));
    sender.join();
    CHECK(received == big);
  }
  SUBCASE("recv_packet на закрытом сокете возвращает false") {
    SockPair sp;
    ::close(sp.a); // закрываем пишущий конец
    std::vector<unsigned char> buf;
    bool ok = recv_packet(sp.b, buf);
    CHECK(ok == false);
    sp.a = -1; // не закрывать повторно в деструкторе
  }
  SUBCASE("send_packet на закрытом сокете возвращает false") {
    SockPair sp;
    ::close(sp.b);
    std::vector<unsigned char> payload(1, 0x00);
    struct sigaction sa{}, old{};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, &old);
    bool norm = send_packet(sp.a, payload);
    CHECK(norm == false);
    sigaction(SIGPIPE, &old, nullptr);
    sp.b = -1;
  }
}

TEST_CASE("send_packet / recv_packet — многопоточный обмен") {
  SUBCASE("producer/consumer в разных потоках") {
    SockPair sp;
    std::vector<unsigned char> sent = {1, 2, 3, 4, 5};
    std::vector<unsigned char> received;
    bool recv_ok = false;

    std::thread producer([&] {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      send_packet(sp.a, sent);
    });
    recv_ok = recv_packet(sp.b, received);
    producer.join();

    CHECK(recv_ok);
    CHECK(received == sent);
  }
}

//  7. Crypto + packet — интеграционный round-trip через сокеты
TEST_CASE("Crypto + packet — зашифрованный обмен через socketpair") {
  SUBCASE("dialer шифрует, receiver расшифровывает") {
    SockPair sp;
    Crypto dialer, receiver;
    std::vector<unsigned char> pub_d(
        dialer.pub_key(), dialer.pub_key() + crypto_kx_PUBLICKEYBYTES);
    std::vector<unsigned char> pub_r(
        receiver.pub_key(), receiver.pub_key() + crypto_kx_PUBLICKEYBYTES);
    REQUIRE(send_packet(sp.a, pub_d));
    REQUIRE(send_packet(sp.b, pub_r));
    std::vector<unsigned char> got_d, got_r;
    REQUIRE(recv_packet(sp.b, got_d));
    REQUIRE(recv_packet(sp.a, got_r));
    dialer.setup_as_dialer(got_r.data());
    receiver.setup_as_receiver(got_d.data());
    std::string text = "секретное сообщение 42";
    auto cipher = dialer.encrypt(text);
    REQUIRE(send_packet(sp.a, cipher));
    std::vector<unsigned char> cipher_recv;
    REQUIRE(recv_packet(sp.b, cipher_recv));
    std::string plain = receiver.decrypt(cipher_recv);
    CHECK(plain == text);
  }
  SUBCASE("несколько сообщений подряд") {
    SockPair sp;
    Crypto dialer, receiver;
    std::vector<unsigned char> pub_d(
        dialer.pub_key(), dialer.pub_key() + crypto_kx_PUBLICKEYBYTES);
    std::vector<unsigned char> pub_r(
        receiver.pub_key(), receiver.pub_key() + crypto_kx_PUBLICKEYBYTES);
    send_packet(sp.a, pub_d);
    send_packet(sp.b, pub_r);
    std::vector<unsigned char> gd, gr;
    recv_packet(sp.b, gd);
    recv_packet(sp.a, gr);
    dialer.setup_as_dialer(gr.data());
    receiver.setup_as_receiver(gd.data());
    std::vector<std::string> messages = {"первое", "второе", "третье",
                                         "четвёртое"};
    for (auto &m : messages) {
      auto c = dialer.encrypt(m);
      REQUIRE(send_packet(sp.a, c));
      std::vector<unsigned char> buf;
      REQUIRE(recv_packet(sp.b, buf));
      CHECK(receiver.decrypt(buf) == m);
    }
  }
}