cmake_minimum_required(VERSION 3.15)
project(Tema_3)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/build/Release/generators")
find_package(libconfig REQUIRED)

add_executable(app1 execv.c)
add_executable(app2 execve.c)
add_executable(app3 execvp.c)
add_executable(app4 execl.c)
add_executable(app5 execle.c)
add_executable(app6 execlp.c)

add_executable(shell shell.c)

target_link_libraries(shell libconfig::libconfig)