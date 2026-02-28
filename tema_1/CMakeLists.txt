cmake_minimum_required(VERSION 3.15)
project(Tema_1)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/Release/generators")
find_package(argtable3 REQUIRED)

add_executable(yell app1.c)
add_executable(vrand app2.c)
add_executable(first app3.c)

target_link_libraries(first argtable3::argtable3)