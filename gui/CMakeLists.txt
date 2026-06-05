cmake_minimum_required(VERSION 3.16)
project(messenger_gui)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---------------------------------------------------------------------------
# Пути установки — пользователь задаёт через -DCMAKE_INSTALL_PREFIX=...
# По умолчанию: <build_dir>/install
# ---------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "Путь установки (задайте свой через -DCMAKE_INSTALL_PREFIX=...)" FORCE)
endif()

set(INSTALL_BIN_DIR "bin"
    CACHE STRING "Подпапка для исполняемых файлов (относительно CMAKE_INSTALL_PREFIX)")

# ---------------------------------------------------------------------------
# Qt6
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
message(STATUS "Qt6: ${Qt6_VERSION}")

# ---------------------------------------------------------------------------
# libsodium: сначала vcpkg, потом pkg-config
# ---------------------------------------------------------------------------
find_package(unofficial-sodium CONFIG QUIET)
if(unofficial-sodium_FOUND)
    message(STATUS "libsodium: найдена через vcpkg")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED libsodium)
    message(STATUS "libsodium: найдена через pkg-config")
endif()

function(link_sodium target)
    if(unofficial-sodium_FOUND)
        target_link_libraries(${target} PRIVATE unofficial-sodium::sodium)
    else()
        target_include_directories(${target} PRIVATE ${SODIUM_INCLUDE_DIRS})
        target_link_directories(${target} PRIVATE ${SODIUM_LIBRARY_DIRS})
        target_link_libraries(${target} PRIVATE ${SODIUM_LIBRARIES})
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Исходники — пути относительные, разделитель /
# ---------------------------------------------------------------------------
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
)

target_link_libraries(messenger_gui PRIVATE
    Qt6::Core
    Qt6::Widgets
)
link_sodium(messenger_gui)

install(TARGETS messenger_gui RUNTIME DESTINATION ${INSTALL_BIN_DIR})
