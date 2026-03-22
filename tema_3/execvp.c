#include <stdio.h>      // perror()
#include <stdlib.h>     // exit(), abort()
#include <unistd.h>     // fork(), execvp(), pid_t
#include <sys/wait.h>   // waitpid(), WEXITSTATUS()
#include <sys/types.h>  // pid_t

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: execvp.c
 * Programul va folosi execvp pentru a apela functia de sistem ps cu argumentul aux
 * pentru a afisa toate procesele care ruleaza pe sistem
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
        // creem lista de argumente, vom folosi comanda ps cu argumentul aux
        // pentru a vedea toate procesele care ruleaza pe sistem
        static char* args[] = {"ps", "aux", NULL};
        // folosim functia execvp cu numele utilitarei ps pe care o va cauta in PATH si lista de argumente
        // fata de execv, aici nu este nevoie sa trimitem calea absoluta
        execvp("ps", args);
        perror("Eroare la apelul execvp()");
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

root@f944939cbc1b:/app# ./main 
USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root         1  0.0  0.0   4168  3412 pts/0    Ss+  09:42   0:00 /bin/bash
root         7  0.0  0.0   4168  3440 pts/1    Ss   09:42   0:00 /bin/bash
root        41  0.0  0.0   2132  1008 pts/1    S+   10:08   0:00 ./main
root        42  0.0  0.0   6008  3252 pts/1    R+   10:08   0:00 ps aux

*/