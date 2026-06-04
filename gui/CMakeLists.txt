cmake_minimum_required(VERSION 3.16)
project(messenger_gui)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/../src/tcp_client.cpp
    ${CMAKE_SOURCE_DIR}/../src/tcp_server.cpp
    ${CMAKE_SOURCE_DIR}/../src/packet.cpp
)

set(GUI_SOURCES
    src/main_gui.cpp
    src/main_window.cpp
    src/connect_dialog.cpp
    src/chat_client.cpp
)

add_executable(messenger_gui
    ${CORE_SOURCES}
    ${GUI_SOURCES}
    include/chat_client.h
    include/connect_dialog.h
    include/main_window.h
)

target_include_directories(messenger_gui PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/../include
    ${SODIUM_INCLUDE_DIRS}
)

target_link_directories(messenger_gui PRIVATE
    ${SODIUM_LIBRARY_DIRS}
)

target_link_libraries(messenger_gui PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${SODIUM_LIBRARIES}
)
