#pragma once

#include "../../include/tcp_client.h"
#include <QObject>
#include <QString>
#include <thread>

/**
 * @brief Адаптер между TCP-клиентом и интерфейсом Qt.
 *
 * Оборачивает класс TcpClient и преобразует сетевые события
 * в сигналы Qt, которые могут использоваться графическим интерфейсом.
 */
class ChatClient : public QObject {
  Q_OBJECT

private:
  /** @brief Объект сетевого клиента. */
  TcpClient client_;

  /** @brief Поток для приёма входящих сообщений. */
  std::thread recvThread_;

  /** @brief Флаг активности потока приёма сообщений. */
  std::atomic_bool running_{false};

  /**
   * @brief Цикл приёма сообщений.
   *
   * Выполняется в отдельном потоке и ожидает новые сообщения
   * от сервера.
   */
  void recvLoop();

public:
  /**
   * @brief Создаёт объект ChatClient.
   *
   * @param parent Родительский объект Qt.
   */
  explicit ChatClient(QObject *parent = nullptr);

  /**
   * @brief Освобождает ресурсы клиента.
   *
   * Завершает поток приёма сообщений и закрывает соединение.
   */
  ~ChatClient();

  /**
   * @brief Подключается к серверу.
   *
   * Создаёт сокет, устанавливает соединение и запускает
   * поток обработки сообщений.
   *
   * @param host IP-адрес сервера.
   * @param port Порт сервера.
   * @return Пустая строка при успехе или текст ошибки.
   */
  QString connectToServer(const QString &host, int port);

  /**
   * @brief Отправляет сообщение на сервер.
   *
   * @param username Имя отправителя.
   * @param text Текст сообщения.
   * @return true если сообщение успешно отправлено.
   * @return false в случае ошибки.
   */
  bool sendMessage(const QString &username, const QString &text);

  /**
   * @brief Закрывает соединение с сервером.
   *
   * Останавливает поток получения сообщений и освобождает ресурсы.
   */
  void disconnect();

signals:
  /**
   * @brief Сигнал нового входящего сообщения.
   *
   * @param msg Полученный текст сообщения.
   */
  void messageReceived(const QString &msg);

  /**
   * @brief Сигнал потери соединения с сервером.
   */
  void connectionLost();
};