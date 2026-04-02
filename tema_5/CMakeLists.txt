cmake_minimum_required(VERSION 3.15)
project(Tema_5)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/Release/generators")

add_executable(server server1.c)
add_executable(client client1.c)