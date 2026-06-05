# Messenger

Мессенджер на C++17 с графическим интерфейсом на Qt и шифрованием сообщений с использованием библиотеки libsodium.

## Назначение

Программа позволяет двум пользователям обмениваться сообщениями через TCP-сервер. Передача сообщений осуществляется в зашифрованном виде с использованием алгоритмов библиотеки libsodium.

## Основные возможности

- подключение к удалённому серверу;
- обмен сообщениями в реальном времени;
- сквозное шифрование сообщений (End-to-End Encryption);
- графический интерфейс на Qt;
- модульное тестирование с помощью Doctest;
- автоматическая документация Doxygen.

## Структура проекта

- `Crypto` — генерация ключей, шифрование и расшифровка сообщений;
- `Packet` — передача пакетов по сети;
- `TcpClient` — клиентская часть;
- `TcpServer` — серверная часть;
- `ChatClient` — адаптер между сетевой частью и Qt;
- `ConnectDialog` — окно подключения;
- `MainWindow` — главное окно приложения.

## Требования

- C++17-совместимый компилятор;
- CMake 3.16 или новее;
- vcpkg (зависимости libsodium, doctest, Qt6 устанавливаются автоматически).

## Установка и сборка

### macOS

**1. Установить Homebrew**

/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

**2. Установить зависимости для сборки**

brew install autoconf autoconf-archive automake libtool make

**3. Клонировать и собрать vcpkg**

git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh

**4. Задать переменные окружения**

export VCPKG_ROOT=~/vcpkg
export PATH="/opt/homebrew/opt/make/libexec/gnubin:$PATH"

**5. Клонировать репозиторий**

git clone https://github.com/mamuraveva/messenger.git
cd messenger

**6. Собрать проект**

cmake --preset vcpkg
cmake --build build

### Ubuntu/Debian

**1. Установить зависимости**

sudo apt install cmake g++ pkg-config libsodium-dev

**2. Клонировать репозиторий**

git clone https://github.com/mamuraveva/messenger.git
cd messenger

**3. Собрать проект**

cmake -B build
cmake --build build

## Запуск

### Сервер

./build/server 8080

Для запуска на VPS рекомендуется использовать tmux:

tmux new -s messenger
./build/server 8080

Отсоединиться без остановки сервера: `Ctrl+B`, затем `D`

Вернуться в сессию:

tmux attach -t messenger

### Клиент

./build/messenger_gui

После запуска:

1. Ввести имя пользователя.
2. Указать IP-адрес сервера и порт.
3. Нажать «Подключиться».
4. Отправлять и получать зашифрованные сообщения.

## Скриншоты

![Окно подключения](docs/connect.png)

![Окно чата](docs/chat.png)

## Тестирование

ctest --test-dir build

## Документация

Сгенерировать документацию:

doxygen Doxyfile

Открыть главную страницу: `docs/html/index.html`

## Автор

Мария Муравьева

