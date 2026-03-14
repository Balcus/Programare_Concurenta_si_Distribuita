#include <stdio.h>  // snprintf(), fopen(), fclose(), fread(), printf(), fprintf(), perror(), size_t
#include <regex.h>  // regcomp(), regexec(), regfree(), regex_t
#include <dirent.h> // opendir(), readdir(), closedir(), DIR, struct dirent
#include <stdlib.h> // malloc(), free(), exit()
#include <string.h> // strstr(), strdup(), strlen()

#define _POSIX_C_SOURCE 200809L // necesar pentru strdup(), altfel primes warning la clang-tidy

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 2: app2.c
 * Ideea de implementarea a utilitarei ps folosind /proc este urmatoarea:
 * - citim toate directoarele aflate in /proc
 * - in cazul in care numele directorului este format doar din cifre atunci el reprezinta un process si il trimitem mai departe la funcita de print_process()
 * - functia print_process() va forma, folosind numele directorului, calea catre fisierul status in formatul /proc/[pid]/status si va citi contentul fisierului
 * - dupa citire, funcita print_process() va trimite datele fisierului status funcitei parse(), care il va parsa si va returna un sir de caractere reprezentand linia cu informatiile: nume ppid, uid, euid, gid, egid ale procesului
 * - functia print_process() va afisa apoi linia returnata de parse() pentru fiecare proces identificat
*/

#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 4096

char* parse(char* file_content) {
    // variabila in care vom retine daca a existat vreo eroare
    int error = 0;

    // initializam informatiile despre procese
    char pname[BUFFER_SIZE] = "Necunoscut";
    int ppid = -1, uid = -1, euid = -1, gid = -1, egid = -1;

    // pointer spre inceputul unui sir de caractere pe care il cautam in continutul fisierului status
    char *res;

    // in cazul in care gasim substring-ul Name: in fisier, strstr v-a returna un pointer catre primul caracter si anume 'N'
    // daca pointer-ul nu e null inseamna ca substring-ul a fost gasit si folosim sscanf pentru a salva sirul de caractere care urmeaza dupa Name:\t in variabila pname
    if ((res = strstr(file_content, "Name:")) != NULL) {
        if (sscanf(res, "Name:\t%s", pname) != 1) {
            // in cazul in care sscanf esueaza, setam error la 1 si continuam
            error = 1;
        }
    }

    // acceasi explicatie ca si mai sus doar ca aici cautam substring-ul PPid: si salvam un numar in loc de sir de caractere
    if ((res = strstr(file_content, "PPid:")) != NULL) {
        if (sscanf(res, "PPid:\t%d", &ppid) != 1) {
            error = 1;
        }
    }

    // acceasi explicatie ca si mai sus doar ca aici cautam substring-ul Uid: si salvam doua numarere in variabilele uid respectiv euid in loc de sir de caractere
    if ((res = strstr(file_content, "Uid:")) != NULL) {
        if (sscanf(res, "Uid:\t%d\t%d", &uid, &euid) < 2) {
            error = 1;
        }
    }

    // acceasi explicatie ca si mai sus doar ca aici cautam substring-ul Uid: si salvam doua numarere in variabilele gid respectiv egid in loc de sir de caractere
    if ((res = strstr(file_content, "Gid:")) != NULL) {
        if (sscanf(res, "Gid:\t%d\t%d", &gid, &egid) < 2) {
            error = 1;
        }
    }

    // in cazul in care am avut o eroare in loc de linia pentru proces, returnam o linie care sa simbolizeze eroarea
    if (error == 1) {
        return strdup("Eroare la parsarea statusului procesului");
    }

    // alocam memoria necesara pentru linie, o formatam si apoi o returnam
    char* line = malloc(BUFFER_SIZE);
    if (line != 0) {
        (void)snprintf(line, BUFFER_SIZE, "Nume: %s, PPID: %d, UID: %d, EUID: %d, GID: %d, EGID: %d", pname, ppid, uid, euid, gid, egid);
    }
    return line;
}

void print_process(struct dirent* entry) {
    // variabila folosita pentru formarea caii catre fisierul status al procesului
    char status_path[BUFFER_SIZE];
    // variabila in care salvam numele procesului 
    char* pid = entry->d_name;
    // formatam calea catre proces si o salvam in variabila status_path
    (void)snprintf(status_path, sizeof(status_path), "/proc/%s/status", pid);

    // deschidem fisierul cu numele /proc/[pid]/status
    FILE* file ;
    if ((file = fopen(status_path, "r")) == NULL) {
        return;
    }

    // alocam memorie pentru a citi datele din fisier in variabila file content
    char* file_content;
    // in cazul in care alocarea de memorie are eroare, incercam sa inchidem fisierul si sa afisam erorile
    if((file_content = malloc(FILE_BUFFER_SIZE)) == NULL) {
        if(fclose(file) != 0) {
            if (fprintf(stderr, "Eroare inchiderea fisierului\n") < 0) {
                perror("Eroare la scrierea in stderr\n");
            }
        }
        if (fprintf(stderr, "Eroare eroare la alocarea de memorie\n") < 0) {
            perror("Eroare la scrierea in stderr\n");
        }
        return;
    }

    // citim continutul fisierului in file_content
    size_t bytes = fread(file_content, 1, FILE_BUFFER_SIZE - 1, file);
    // adaugam la final terminatorul null
    file_content[bytes] = '\0';
    // trimitem continutul la functia parse() si retinem rezultatul in variabila parsed
    char* parsed = parse(file_content);
    // in cazul in care functia parse a returnat o linie, o afisam si apoi eliberam resursele pentru acea linie
    if (parsed) {
        printf("PID %s: %s\n", pid, parsed);
        free(parsed);
    }
    // eliberam resursele pentru continutul fisierului
    free(file_content);
    // incercam sa inchidem fisierul si tratam erorile
    if(fclose(file) != 0) {
        if (fprintf(stderr, "Eroare inchiderea fisierului\n") < 0) {
            perror("Eroare la scrierea in stderr\n");
        }
    }
}

int main() {
    // variabila pentru retinerea fiecarui entry din /proc
    struct dirent *entry;
    // variabila care retine rezultatul opendir("/proc")
    DIR* dir;

    // variabila pentru expresia regex
    regex_t regex;
    // compilam expresia regex care sa identifice sirurile de caractere formate doar din numere si tratam erorile in cazul in care apar afisandu-le
    // (folosim REG_EXTENDED deoarece, fara el, '+' ar fi luat ca si caracter propriu-zis, nu ca si 'unul sau mai multe')
    if (regcomp(&regex, "^[0-9]+$", REG_EXTENDED) != 0) {
        if (fprintf(stderr, "Eroare la compilare regex\n") < 0) {
            perror("Eroare la scrierea in stderr\n");
        }
        exit(1);
    }

    // deschidem directorul /proc si tratam erorile in cazul in care apar afisandu-le
    if ((dir = opendir("/proc")) == NULL) {
        if (fprintf(stderr, "Eroare la deschiderea directorului /proc\n") < 0) {
            perror("Eroare la scrierea in stderr\n");
        }
        exit(1);
    }

    // citim fiecare entry din directorul /proc
    while ((entry = readdir(dir)) != NULL) {
        // in cazul in care numele nu respecta expresia regex continuam la urmatorul entry
        if (regexec(&regex, entry->d_name, 0, NULL, 0) != 0) {
            continue;
        }
        // in cazul in care respecta expresia regex inseamna ca am gasit un director proces si trimitem entry-ul la funcita print_process()
        print_process(entry);
    }

    // eliberam resursele folosite de dir si expresia regex
    closedir(dir);
    regfree(&regex);

    return 0;
}

/*

root@f944939cbc1b:/app# clang-tidy mps.c -- -std=c17
5 warnings generated.
Suppressed 5 warnings (5 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.

root@f944939cbc1b:/app# ps -ef
UID        PID  PPID  C STIME TTY          TIME CMD
root         1     0  0 17:17 pts/0    00:00:00 /bin/bash
root         7     0  0 17:17 pts/1    00:00:00 /bin/bash
root       173     7  0 18:25 pts/1    00:00:00 ps -ef

root@f944939cbc1b:/app# ./main 
PID 1: Nume: bash, PPID: 0, UID: 0, EUID: 0, GID: 0, EGID: 0
PID 7: Nume: bash, PPID: 0, UID: 0, EUID: 0, GID: 0, EGID: 0
PID 174: Nume: main, PPID: 7, UID: 0, EUID: 0, GID: 0, EGID: 0

root@f944939cbc1b:/app# sleep 10000 &
[1] 178

root@f944939cbc1b:/app# sleep 10000 &
[2] 180

root@f944939cbc1b:/app# ps -ef
UID        PID  PPID  C STIME TTY          TIME CMD
root         1     0  0 17:17 pts/0    00:00:00 /bin/bash
root         7     0  0 17:17 pts/1    00:00:00 /bin/bash
root       178     7  0 18:26 pts/1    00:00:00 sleep 10000
root       180     7  0 18:26 pts/1    00:00:00 sleep 10000
root       182     7  0 18:26 pts/1    00:00:00 ps -ef

root@f944939cbc1b:/app# ./main 
PID 1: Nume: bash, PPID: 0, UID: 0, EUID: 0, GID: 0, EGID: 0
PID 7: Nume: bash, PPID: 0, UID: 0, EUID: 0, GID: 0, EGID: 0
PID 178: Nume: sleep, PPID: 7, UID: 0, EUID: 0, GID: 0, EGID: 0
PID 180: Nume: sleep, PPID: 7, UID: 0, EUID: 0, GID: 0, EGID: 0
PID 183: Nume: main, PPID: 7, UID: 0, EUID: 0, GID: 0, EGID: 0

*/