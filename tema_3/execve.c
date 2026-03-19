#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: execve.c
 * Programul va folosi execve pentru a apela functia de sistem echo intr-un 
 * nou sehll si a afisa o varabila de mediu trimisa ca argument numita MESSAGE
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
        static char* args[] = {"sh", "-c", "echo $MESSAGE", NULL};
        static char* environ[] = { "MESSAGE=hello", NULL };
        execve("/bin/sh", args, environ);
        perror("Eroare la apelul execve()");
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

% ./build/app2
hello

*/