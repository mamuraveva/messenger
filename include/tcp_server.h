#pragma once

#include <mutex>
#include <string>
#include <vector>

/**
 * @brief TCP-сервер для подключения клиентов и пересылки сообщений.
 *
 * Сервер принимает подключения, выполняет обмен ключами между
 * клиентами и пересылает зашифрованные сообщения между ними.
 */
class TcpServer {
private:
  /** @brief Дескриптор серверного сокета. */
  int server_socket = -1;

  /** @brief Список подключённых клиентов. */
  std::vector<int> clients;

  /** @brief Мьютекс для синхронизации доступа к списку клиентов. */
  std::mutex clients_mutex;

public:
  /**
   * @brief Запускает сервер.
   *
   * Создаёт сокет, привязывает его к порту и начинает ожидание клиентов.
   *
   * @param port Порт для прослушивания.
   * @return true если сервер завершил работу корректно.
   * @return false в случае ошибки.
   */
  bool start(int port);

  /**
   * @brief Проверяет корректность номера порта.
   *
   * @param port Проверяемый порт.
   * @return true если порт находится в допустимом диапазоне.
   * @return false если порт некорректен.
   */
  bool validatePort(int port);

  /**
   * @brief Принимает подключения клиентов.
   *
   * Ожидает подключения двух клиентов, выполняет обмен
   * публичными ключами и запускает потоки обработки.
   */
  void accept_clients();

  /**
   * @brief Обрабатывает сообщения одного клиента.
   *
   * Получает пакеты от клиента и пересылает их остальным участникам.
   *
   * @param client_socket Сокет клиента.
   */
  void handle_client(int client_socket);

  /**
   * @brief Пересылает сообщение всем клиентам кроме отправителя.
   *
   * @param data Передаваемый пакет данных.
   * @param sender_socket Сокет отправителя сообщения.
   */
  void broadcast_message(const std::vector<unsigned char> &data,
                         int sender_socket);
};