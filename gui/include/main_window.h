#pragma once

#include "chat_client.h"
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>

/**
 * @brief Главное окно мессенджера.
 *
 * Отображает историю сообщений, состояние подключения
 * и предоставляет интерфейс для отправки сообщений.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

private:
  /**
   * @brief Добавляет сообщение в историю чата.
   *
   * @param msg Текст сообщения.
   * @param isMine true для собственных сообщений,
   * false для сообщений собеседника.
   */
  void addMessage(const QString &msg, bool isMine = false);

  /**
   * @brief Настраивает внешний вид интерфейса.
   */
  void setupStyle();

  /** @brief Имя текущего пользователя. */
  QString username_;

  /** @brief Объект для работы с сетевым клиентом. */
  ChatClient *chatClient_;

  /** @brief Список сообщений чата. */
  QListWidget *messageList_;

  /** @brief Поле ввода текста сообщения. */
  QLineEdit *inputEdit_;

  /** @brief Кнопка отправки сообщения. */
  QPushButton *sendBtn_;

  /** @brief Метка состояния подключения. */
  QLabel *statusLabel_;

public:
  /**
   * @brief Создаёт главное окно мессенджера.
   *
   * @param username Имя пользователя.
   * @param parent Родительский объект Qt.
   */
  explicit MainWindow(const QString &username,
                      QWidget *parent = nullptr);

  /**
   * @brief Освобождает ресурсы окна.
   */
  ~MainWindow();

  /**
   * @brief Подключается к серверу.
   *
   * @param host IP-адрес сервера.
   * @param port Порт сервера.
   * @return Пустая строка при успехе или текст ошибки.
   */
  QString connectToServer(const QString &host, int port);

private slots:
  /**
   * @brief Обрабатывает отправку сообщения.
   */
  void onSend();

  /**
   * @brief Обрабатывает получение нового сообщения.
   *
   * @param msg Полученное сообщение.
   */
  void onMessageReceived(const QString &msg);

  /**
   * @brief Обрабатывает потерю соединения с сервером.
   */
  void onConnectionLost();
};