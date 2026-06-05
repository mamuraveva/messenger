cmake_minimum_required(VERSION 3.16)
project(cppproject)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Пути установки — пользователь задаёт через -DCMAKE_INSTALL_PREFIX=...
# По умолчанию: <build_dir>/install
# Все пути относительные, разделитель /
# ---------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "Путь установки (задайте свой через -DCMAKE_INSTALL_PREFIX=...)" FORCE)
endif()

# Относительные подпапки внутри prefix (можно переопределить при конфигурации)
set(INSTALL_BIN_DIR "bin"
    CACHE STRING "Подпапка для исполняемых файлов (относительно CMAKE_INSTALL_PREFIX)")

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
# SERVER
# ---------------------------------------------------------------------------
add_executable(server
    src/server_main.cpp
    src/tcp_server.cpp
    src/packet.cpp
)
target_include_directories(server PRIVATE include)
link_sodium(server)
install(TARGETS server RUNTIME DESTINATION ${INSTALL_BIN_DIR})

# ---------------------------------------------------------------------------
# CLIENT
# ---------------------------------------------------------------------------
add_executable(client
    src/client_main.cpp
    src/tcp_client.cpp
    src/packet.cpp
)
target_include_directories(client PRIVATE include)
link_sodium(client)
install(TARGETS client RUNTIME DESTINATION ${INSTALL_BIN_DIR})

# ---------------------------------------------------------------------------
# TESTS
# Doctest: vcpkg target → иначе локальный заголовок tests/doctest.h
# ---------------------------------------------------------------------------
find_package(doctest CONFIG QUIET)

add_executable(tests
    tests/tests.cpp
    src/tcp_client.cpp
    src/tcp_server.cpp
    src/packet.cpp
)
target_include_directories(tests PRIVATE include tests)
link_sodium(tests)

if(doctest_FOUND)
    message(STATUS "doctest: найден через vcpkg")
    target_link_libraries(tests PRIVATE doctest::doctest)
else()
    message(STATUS "doctest: используется локальный заголовок tests/doctest.h")
endif()

enable_testing()
add_test(NAME all_tests COMMAND tests)
