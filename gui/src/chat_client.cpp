#include "chat_client.h"
#include <QMetaObject>
 
ChatClient::ChatClient(QObject *parent) : QObject(parent) {
    client_.on_message_ = [this](const std::string &msg) {
        QString qmsg = QString::fromStdString(msg);
        QMetaObject::invokeMethod(
            this,
            [this, qmsg]() { emit messageReceived(qmsg); },
            Qt::QueuedConnection);
    };
}
 
ChatClient::~ChatClient() {
    disconnect();
}
 
QString ChatClient::connectToServer(const QString &host, int port) {
    if (!client_.create()) {
        return "не удалось создать сокет";
    }
 
    if (!client_.connect(host.toStdString(), port)) {
        return "не удалось подключиться к серверу";
    }
 
    if (!client_.do_handshake()) {
        return "ошибка установки зашифрованного соединения";
    }
 
    running_ = true;
 
    recvThread_ = std::thread([this]() {
        client_.receive_loop();
 
        if (running_) {
            QMetaObject::invokeMethod(
                this,
                [this]() { emit connectionLost(); },
                Qt::QueuedConnection);
        }
    });
 
    return "";
}
 
bool ChatClient::sendMessage(const QString &username, const QString &text) {
    return client_.send_message(username.toStdString() + ": " +
                                text.toStdString());
}
 
void ChatClient::disconnect() {
    running_ = false;
    client_.close();
 
    if (recvThread_.joinable()) {
        recvThread_.join();
    }
}
 
