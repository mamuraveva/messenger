#include "main_window.h"
#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QScrollBar>
#include <QVBoxLayout>

MainWindow::MainWindow(const QString &username, QWidget *parent)
: QMainWindow(parent), username_(username) {
setWindowTitle("messenger — " + username);
setMinimumSize(500, 600);
resize(520, 660);
chatClient_ = new ChatClient(this);
connect(chatClient_, &ChatClient::messageReceived, this,
&MainWindow::onMessageReceived);
connect(chatClient_, &ChatClient::connectionLost, this,
&MainWindow::onConnectionLost);
auto *central = new QWidget(this);
setCentralWidget(central);

// шапка, список сообщений, поле ввода.
auto *root = new QVBoxLayout(central);
root->setContentsMargins(0, 0, 0, 0);
root->setSpacing(0);

// Название приложения и статус подключения.
auto *header = new QWidget(this);
header->setObjectName("header");
header->setFixedHeight(52);
auto *headerLayout = new QHBoxLayout(header);
headerLayout->setContentsMargins(20, 0, 20, 0);
auto *appName = new QLabel("messenger", header);
appName->setObjectName("appName");
statusLabel_ = new QLabel("онлайн", header);
statusLabel_->setObjectName("status");
statusLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
headerLayout->addWidget(appName);
headerLayout->addStretch();
headerLayout->addWidget(statusLabel_);
root->addWidget(header);
// Список сообщений для хранения истории переписки
messageList_ = new QListWidget(this);
messageList_->setObjectName("messageList");
messageList_->setSelectionMode(QAbstractItemView::NoSelection);
messageList_->setFocusPolicy(Qt::NoFocus);
messageList_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
messageList_->verticalScrollBar()->setSingleStep(8);
root->addWidget(messageList_, 1);
// Панель между историей сообщений и панелью ввода
auto *divider = new QWidget(this);
divider->setObjectName("divider");
divider->setFixedHeight(1);
root->addWidget(divider);
// Нижняя панель ввода сообщения
auto *inputBar = new QWidget(this);
inputBar->setObjectName("inputBar");
inputBar->setFixedHeight(64);
auto *inputLayout = new QHBoxLayout(inputBar);
inputLayout->setContentsMargins(16, 12, 16, 12);
inputLayout->setSpacing(10);
inputEdit_ = new QLineEdit(inputBar);
inputEdit_->setObjectName("inputEdit");
inputEdit_->setPlaceholderText("сообщение...");
inputEdit_->setClearButtonEnabled(true);
sendBtn_ = new QPushButton("→", inputBar);
sendBtn_->setObjectName("sendBtn");
sendBtn_->setFixedSize(40, 40);
sendBtn_->setCursor(Qt::PointingHandCursor);
inputLayout->addWidget(inputEdit_);
inputLayout->addWidget(sendBtn_);
root->addWidget(inputBar);
// Сообщение можно отправить кнопкой или клавишей Enter
connect(sendBtn_, &QPushButton::clicked, this, &MainWindow::onSend);
connect(inputEdit_, &QLineEdit::returnPressed, this, &MainWindow::onSend);
setupStyle();
}
MainWindow::~MainWindow() { chatClient_->disconnect(); }
QString MainWindow::connectToServer(const QString &host, int port) {
return chatClient_->connectToServer(host, port);
}
void MainWindow::onSend() {
QString text = inputEdit_->text().trimmed();
if (text.isEmpty()) {
return;
}
if (!chatClient_->sendMessage(username_, text)) {
statusLabel_->setText("ошибка отправки");
return;
}
addMessage(text, true);
inputEdit_->clear();
}
void MainWindow::onMessageReceived(const QString &msg) {
addMessage(msg, false);
}
void MainWindow::onConnectionLost() {
statusLabel_->setText("соединение потеряно");
statusLabel_->setObjectName("statusOffline");
statusLabel_->style()->unpolish(statusLabel_);
statusLabel_->style()->polish(statusLabel_);
inputEdit_->setEnabled(false);
sendBtn_->setEnabled(false);
}
void MainWindow::addMessage(const QString &msg, bool isMine) {
auto *item = new QListWidgetItem(messageList_);

// Разный стиль для своих и чужих сообщений.
auto *bubble = new QWidget();
bubble->setObjectName(isMine ? "bubbleMine" : "bubbleTheirs");
auto *layout = new QVBoxLayout(bubble);
layout->setContentsMargins(12, 8, 12, 8);
layout->setSpacing(2);
auto *text = new QLabel(msg, bubble);
text->setObjectName("bubbleText");
text->setWordWrap(true);
text->setMaximumWidth(320);
// Время добавляется отдельно, чтобы визуально отделять текст сообщения
auto *time =
new QLabel(QDateTime::currentDateTime().toString("hh:mm"), bubble);
time->setObjectName("bubbleTime");
time->setAlignment(isMine ? Qt::AlignRight : Qt::AlignLeft);
layout->addWidget(text);
layout->addWidget(time);
auto *row = new QWidget();
auto *rowLayout = new QHBoxLayout(row);
rowLayout->setContentsMargins(8, 4, 8, 4);
if (isMine) {
rowLayout->addStretch();
rowLayout->addWidget(bubble);
} else {
rowLayout->addWidget(bubble);
rowLayout->addStretch();
}

item->setSizeHint(row->sizeHint());
messageList_->setItemWidget(item, row);
messageList_->scrollToBottom();
}
void MainWindow::setupStyle() {
qApp->setStyleSheet(R"(
QWidget {
background-color: #0f0f0f;
color: #e8e8e8;
font-family: "Helvetica Neue", sans-serif;
font-size: 14px;
}
    QWidget#header {
        background-color: #0f0f0f;
        border-bottom: 1px solid #1a1a1a;
    }
    QLabel#appName {
        font-size: 16px;
        font-weight: 600;
        letter-spacing: 2px;
        color: #ffffff;
        background: transparent;
    }
    QLabel#status {
        font-size: 11px;
        color: #4caf82;
        letter-spacing: 1px;
        background: transparent;
    }
    QLabel#statusOffline {
        font-size: 11px;
        color: #e05252;
        letter-spacing: 1px;
        background: transparent;
    }
    QListWidget#messageList {
        background-color: #0f0f0f;
        border: none;
        outline: none;
    }
    QListWidget#messageList::item {
        background: transparent;
        border: none;
        padding: 0;
    }
    QListWidget#messageList QWidget {
        background: transparent;
    }
    QScrollBar:vertical {
        background: transparent;
        width: 4px;
    }
    QScrollBar::handle:vertical {
        background: #2a2a2a;
        border-radius: 2px;
    }
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {
        height: 0;
    }
    QWidget#bubbleMine {
        background-color: #1e3a2f;
        border-radius: 16px;
        border-bottom-right-radius: 4px;
    }
    QWidget#bubbleTheirs {
        background-color: #1e1e1e;
        border-radius: 16px;
        border-bottom-left-radius: 4px;
    }
    QLabel#bubbleText {
        color: #e8e8e8;
        font-size: 14px;
        background: transparent;
    }
    QLabel#bubbleTime {
        color: #555;
        font-size: 10px;
        background: transparent;
    }
    QWidget#divider {
        background-color: #1a1a1a;
    }
    QWidget#inputBar {
        background-color: #111111;
    }
    QLineEdit#inputEdit {
        background-color: #1a1a1a;
        border: 1px solid #2a2a2a;
        border-radius: 20px;
        padding: 8px 16px;
        color: #e8e8e8;
        font-size: 14px;
        selection-background-color: #2e6b4f;
    }
    QLineEdit#inputEdit:focus {
        border: 1px solid #3a3a3a;
    }
    QPushButton#sendBtn {
        background-color: #2e6b4f;
        border: none;
        border-radius: 20px;
        color: #ffffff;
        font-size: 18px;
        font-weight: bold;
    }
    QPushButton#sendBtn:hover {
        background-color: #3a8560;
    }
    QPushButton#sendBtn:pressed {
        background-color: #245540;
    }
    QPushButton#sendBtn:disabled {
        background-color: #1a1a1a;
        color: #333;
    }
    QDialog {
        background-color: #111111;
    }
    QLabel#title {
        font-size: 22px;
        font-weight: 700;
        letter-spacing: 4px;
        color: #ffffff;
        background: transparent;
    }
    QLabel#error {
        color: #e05252;
        font-size: 12px;
        background: transparent;
    }
    QFormLayout QLabel {
        color: #555;
        font-size: 12px;
        letter-spacing: 1px;
        background: transparent;
    }
    QLineEdit {
        background-color: #1a1a1a;
        border: 1px solid #2a2a2a;
        border-radius: 8px;
        padding: 8px 12px;
        color: #e8e8e8;
        font-size: 14px;
    }
    QLineEdit:focus {
        border: 1px solid #3a3a3a;
    }
    QPushButton#connectBtn {
        background-color: #2e6b4f;
        border: none;
        border-radius: 10px;
        padding: 10px;
        color: #ffffff;
        font-size: 13px;
        letter-spacing: 1px;
    }
    QPushButton#connectBtn:hover {
        background-color: #3a8560;
    }
    QPushButton#connectBtn:pressed {
        background-color: #245540;
    }
)");
}