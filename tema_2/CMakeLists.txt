cmake_minimum_required(VERSION 3.15)
project(Tema_2)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/Release/generators")

add_executable(app1 app1.c)
add_executable(app2 app2.c)
