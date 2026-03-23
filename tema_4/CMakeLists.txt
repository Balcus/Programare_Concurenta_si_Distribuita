cmake_minimum_required(VERSION 3.15)
project(Tema_4)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/Release/generators")

add_executable(file_worker file_worker.c)

find_package(Threads REQUIRED)
target_link_libraries(file_worker Threads::Threads)