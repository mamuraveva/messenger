#pragma once
#include "../../include/tcp_client.h"
#include <QObject>
#include <QString>
#include <thread>

class ChatClient : public QObject {
  Q_OBJECT
private:
  TcpClient client_;
  std::thread recvThread_;
  std::atomic_bool running_{false};
  void recvLoop();
public:
  explicit ChatClient(QObject *parent = nullptr);
  ~ChatClient();
  QString connectToServer(const QString &host, int port);
  bool sendMessage(const QString &username, const QString &text);
  void disconnect();
signals:
  void messageReceived(const QString &msg);
  void connectionLost();
};