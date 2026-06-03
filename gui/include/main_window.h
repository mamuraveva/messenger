#pragma once
#include "chat_client.h"
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>

class MainWindow : public QMainWindow {
  Q_OBJECT
private:
  void addMessage(const QString &msg, bool isMine = false);
  void setupStyle();
  QString username_;
  ChatClient *chatClient_;
  QListWidget *messageList_;
  QLineEdit *inputEdit_;
  QPushButton *sendBtn_;
  QLabel *statusLabel_;
public:
  explicit MainWindow(const QString &username,
                    QWidget *parent = nullptr);
  ~MainWindow();
  QString connectToServer(const QString &host, int port);
private slots:
  void onSend();
  void onMessageReceived(const QString &msg);
  void onConnectionLost();
};