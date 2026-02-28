cmake_minimum_required(VERSION 3.15)
project(Tema_1)

find_package(argtable3 REQUIRED)

add_executable(yell app1.c)
add_executable(vrand app2.c)
add_executable(app3 app3.c)

target_link_libraries(app3 argtable3::argtable3)