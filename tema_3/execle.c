#include <stdio.h>      // perror()
#include <stdlib.h>     // exit(), abort()
#include <unistd.h>     // fork(), execle()
#include <sys/wait.h>   // waitpid(), WEXITSTATUS()
#include <sys/types.h>  // pid_t

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: execle.c
 * Programul va folosi execle 
 * pentru a apela functia de sistem echo intr-un nou shell
 * si a printa variabila de mediu MESSAGE primita ca parametru
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
        static char* environ[] = { "MESSAGE=world", NULL };
        execle("/bin/sh", "sh", "-c", "echo $MESSAGE", NULL, environ);
        perror("Eroare la apelul execle()");
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

./build/app5
world

*/