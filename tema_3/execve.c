#include <stdio.h>      // perror()
#include <stdlib.h>     // exit(), abort()
#include <unistd.h>     // fork(), execve()
#include <sys/wait.h>   // waitpid(), WEXITSTATUS()
#include <sys/types.h>  // pid_t

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: execve.c
 * Programul va folosi execve pentru a apela functia de sistem echo intr-un 
 * nou sehll si a afisa o varabila de mediu trimisa ca argument numita MESSAGE
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
        // creem lista de argumente, vom executa echo la o variab de mediu deci pentru asta avem nevoie de un shell
        // flagul -c spune shell-ului sa execute comanda data sub forma de string
        static char* args[] = {"sh", "-c", "echo $MESSAGE", NULL};
        // creem lista de variabile de mediu, vom trimtie o singura variabila de mediu numita MESSAGE cu valoaread hello
        static char* environ[] = { "MESSAGE=hello", NULL };
        // folosim apelul de sistem execve caruia
        // ii dam calea absoulta spre shell, lista de argumente si cea de variabile de mediu
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