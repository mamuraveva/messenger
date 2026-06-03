#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class ConnectDialog : public QDialog {
  Q_OBJECT
private:
  QLineEdit *usernameEdit_;
  QLineEdit *hostEdit_;
  QLineEdit *portEdit_;
  QPushButton *connectBtn_;
  QLabel *errorLabel_;
public:
  explicit ConnectDialog(QWidget *parent = nullptr);
  QString username() const { return usernameEdit_->text().trimmed(); }
  QString host() const { return hostEdit_->text().trimmed(); }
  int port() const { return portEdit_->text().toInt(); }
private slots:
  void onConnect();
};

