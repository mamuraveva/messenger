#include "connect_dialog.h"
#include <QVBoxLayout>

ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Messenger");
  setFixedSize(320, 320);
  setModal(true);

  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(36, 40, 36, 36);
  root->setSpacing(0);

  auto *title = new QLabel("Messenger", this);
  title->setObjectName("title");
  title->setAlignment(Qt::AlignCenter);
  root->addWidget(title);
  root->addSpacing(28);

  usernameEdit_ = new QLineEdit(this);
  usernameEdit_->setPlaceholderText("Ваше имя");
  usernameEdit_->setAlignment(Qt::AlignCenter);
  root->addWidget(usernameEdit_);
  root->addSpacing(10);

  hostEdit_ = new QLineEdit(this);
  hostEdit_->setPlaceholderText("адрес сервера");
  hostEdit_->setAlignment(Qt::AlignCenter);
  root->addWidget(hostEdit_);
  root->addSpacing(12);

  portEdit_ = new QLineEdit(this);
  portEdit_->setPlaceholderText("порт");
  portEdit_->setText("8080");
  portEdit_->setAlignment(Qt::AlignCenter);
  root->addWidget(portEdit_);
  root->addSpacing(12);

  errorLabel_ = new QLabel(this);
  errorLabel_->setObjectName("error");
  errorLabel_->setAlignment(Qt::AlignCenter);
  errorLabel_->hide();
  root->addWidget(errorLabel_);
  root->addSpacing(12);

  connectBtn_ = new QPushButton("войти", this);
  connectBtn_->setObjectName("connectBtn");
  connectBtn_->setCursor(Qt::PointingHandCursor);
  root->addWidget(connectBtn_);

  connect(connectBtn_, &QPushButton::clicked, this,
          &ConnectDialog::onConnect);

  connect(usernameEdit_, &QLineEdit::returnPressed, this,
          &ConnectDialog::onConnect);

  connect(hostEdit_, &QLineEdit::returnPressed, this,
          &ConnectDialog::onConnect);

  connect(portEdit_, &QLineEdit::returnPressed, this,
          &ConnectDialog::onConnect);
}

void ConnectDialog::onConnect() {
  if (username().isEmpty()) {
    errorLabel_->setText("Введите имя");
    errorLabel_->show();
    return;
  }

  if (host().isEmpty()) {
    errorLabel_->setText("Введите адрес сервера");
    errorLabel_->show();
    return;
  }

  errorLabel_->hide();
  accept();
}