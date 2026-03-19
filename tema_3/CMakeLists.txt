cmake_minimum_required(VERSION 3.15)
project(Tema_2)

add_executable(app1 execv.c)
add_executable(app2 execve.c)
add_executable(app3 execvp.c)
add_executable(app4 execl.c)
add_executable(app5 execle.c)
add_executable(app6 execlp.c)

add_executable(shell shell.c)