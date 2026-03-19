#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: execv.c
 * Programul va folosi execv pentru a apela functia de system ls folosind argumentele -la
*/

int main() {
    pid_t cpid;
    int status = 0;

    cpid = fork();

    if (cpid == -1) {
        perror("Eroare la apelul fork()\n");
        exit(1);
    }

    if (cpid == 0) {
        static char* args[] = {"ls", "-l", "-a", NULL};
        execv("/bin/ls", args);
        perror("Eroare la apelul execv()");
        abort();
    } else {
        if (waitpid(cpid, &status, 0) == -1) {
            perror("Eroare la apelul wait()");
            exit(1);
        }
    }

    return WEXITSTATUS(status);
}

/*

root@f944939cbc1b:/app# clang-tidy main.c -- -std=c17
2 warnings generated.
Suppressed 2 warnings (2 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.

% ./build/app1
total 80
drwxr-xr-x@ 13 balcus  staff  416 Mar 19 11:41 .
drwxr-xr-x@  8 balcus  staff  256 Mar 16 22:05 ..
-rw-r--r--@  1 balcus  staff   25 Mar 16 22:07 .gitignore
-rw-r--r--@  1 balcus  staff  262 Mar 16 22:14 CMakeLists.txt
drwxr-xr-x@ 13 balcus  staff  416 Mar 19 11:41 build
-rw-r--r--@  1 balcus  staff   69 Mar 16 22:10 execl.c
-rw-r--r--@  1 balcus  staff   69 Mar 16 22:10 execle.c
-rw-r--r--@  1 balcus  staff   69 Mar 16 22:10 execlp.c
-rw-r--r--@  1 balcus  staff  607 Mar 19 11:40 execv.c
-rw-r--r--@  1 balcus  staff   69 Mar 16 22:10 execve.c
-rw-r--r--@  1 balcus  staff   69 Mar 16 22:10 execvp.c
-rw-r--r--@  1 balcus  staff  135 Mar 16 22:13 justfile
-rw-r--r--@  1 balcus  staff   69 Mar 16 22:14 shell.c

*/