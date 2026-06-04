#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

/**
 * @brief Диалог подключения к серверу.
 *
 * Запрашивает имя пользователя, IP-адрес сервера
 * и номер порта перед запуском мессенджера.
 */
class ConnectDialog : public QDialog {
  Q_OBJECT

private:
  /** @brief Поле ввода имени пользователя. */
  QLineEdit *usernameEdit_;

  /** @brief Поле ввода адреса сервера. */
  QLineEdit *hostEdit_;

  /** @brief Поле ввода порта сервера. */
  QLineEdit *portEdit_;

  /** @brief Кнопка подключения к серверу. */
  QPushButton *connectBtn_;

  /** @brief Метка для отображения ошибок ввода. */
  QLabel *errorLabel_;

public:
  /**
   * @brief Создаёт диалог подключения.
   *
   * @param parent Родительский объект Qt.
   */
  explicit ConnectDialog(QWidget *parent = nullptr);

  /**
   * @brief Возвращает имя пользователя.
   *
   * @return Имя пользователя без лишних пробелов.
   */
  QString username() const { return usernameEdit_->text().trimmed(); }

  /**
   * @brief Возвращает адрес сервера.
   *
   * @return Строка с IP-адресом сервера.
   */
  QString host() const { return hostEdit_->text().trimmed(); }

  /**
   * @brief Возвращает номер порта.
   *
   * @return Номер порта сервера.
   */
  int port() const { return portEdit_->text().toInt(); }

private slots:
  /**
   * @brief Проверяет корректность введённых данных.
   *
   * При успешной проверке закрывает диалог и подтверждает подключение.
   */
  void onConnect();
};