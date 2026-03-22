#include <stdio.h>      // perror()
#include <stdlib.h>     // exit(), abort()
#include <unistd.h>     // fork(), execlp()
#include <sys/wait.h>   // waitpid(), WEXITSTATUS()
#include <sys/types.h>  // pid_t

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: execlp.c
 * Programul va folosi execlp pentru a apela functia de sistem whoami si a afisa user-ul curent
*/

// nu voi mai adauga exact aceleasi comentarii ca si la execv
// voi adauga doar unde se schimba ceva.
int main() {
    pid_t cpid;
    int status = 0;

    cpid = fork();

    if (cpid == -1) {
        perror("Eroare la apelul fork()\n");
        exit(1);
    }

    if (cpid == 0) {
        // vom folosi execpl pentru a afisa user-ul curent folosind comanda whoami
        // trimitem doar numele comenzii in loc de calea relativa iar execpl o va cauta in PATH
        execlp("whoami", "whoami", NULL);
        perror("Eroare la apelul execlp()");
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

% ./build/app6
balcus

*/