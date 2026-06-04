cmake_minimum_required(VERSION 3.10)
project(cppproject)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# libsodium
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

# SERVER
add_executable(server
    src/server_main.cpp
    src/tcp_server.cpp
    src/packet.cpp
)
target_include_directories(server PRIVATE
    include
    ${SODIUM_INCLUDE_DIRS}
)
target_link_directories(server PRIVATE
    ${SODIUM_LIBRARY_DIRS}
)
target_link_libraries(server PRIVATE
    ${SODIUM_LIBRARIES}
)

# CLIENT
add_executable(client
    src/client_main.cpp
    src/tcp_client.cpp
    src/packet.cpp
)
target_include_directories(client PRIVATE
    include
    ${SODIUM_INCLUDE_DIRS}
)
target_link_directories(client PRIVATE
    ${SODIUM_LIBRARY_DIRS}
)
target_link_libraries(client PRIVATE
    ${SODIUM_LIBRARIES}
)

# TESTS
add_executable(tests
    tests/tests.cpp
    src/tcp_client.cpp
    src/tcp_server.cpp
    src/packet.cpp
)
target_include_directories(tests PRIVATE
    include
    tests
    ${SODIUM_INCLUDE_DIRS}
)
target_link_directories(tests PRIVATE
    ${SODIUM_LIBRARY_DIRS}
)
target_link_libraries(tests PRIVATE
    ${SODIUM_LIBRARIES}
)

enable_testing()
add_test(NAME all_tests COMMAND tests)