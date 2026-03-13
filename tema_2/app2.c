// parse /proc/[0-9]*/status
// page 171
#include <stdio.h>  // snprintf(), fopen(), fclose(), fread(), printf(), fprintf(), perror(), size_t
#include <regex.h>  // regcomp(), regexec(), regfree(), regex_t
#include <dirent.h> // opendir(), readdir(), closedir(), DIR, struct dirent
#include <stdlib.h> // malloc(), free(), exit()
#include <string.h> // strstr(), strdup(), strlen()

#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 4096

char* parse(char* file_content) {
    int error = 0;
    char pname[BUFFER_SIZE] = "Necunoscut";
    int ppid = -1, uid = -1, euid = -1, gid = -1, egid = -1;

    char *res;

    if ((res = strstr(file_content, "Name:")) != NULL) {
        if (sscanf(res, "Name:\t%s", pname) != 1) {
            error = 1;
        }
    }

    if ((res = strstr(file_content, "PPid:")) != NULL) {
        if (sscanf(res, "PPid:\t%d", &ppid) != 1) {
            error = 1;
        }
    }

    if ((res = strstr(file_content, "Uid:")) != NULL) {
        if (sscanf(res, "Uid:\t%d\t%d", &uid, &euid) < 2) {
            error = 1;
        }
    }

    if ((res = strstr(file_content, "Gid:")) != NULL) {
        if (sscanf(res, "Gid:\t%d\t%d", &gid, &egid) < 2) {
            error = 1;
        }
    }

    if (error == 1) {
        return strdup("Eroare la parsarea statusului procesului");
    }

    char* result = malloc(BUFFER_SIZE);
    if (result != 0) {
        snprintf(result, BUFFER_SIZE, "Nume: %s, PPID: %d, UID: %d, EUID: %d, GID: %d, EGID: %d", pname, ppid, uid, euid, gid, egid);
    }
    return result;
}

void print_process(struct dirent* entry) {
    char status_path[BUFFER_SIZE];
    char* pid = entry->d_name;
    snprintf(status_path, sizeof(status_path), "/proc/%s/status", pid);

    FILE* file ;
    if ((file = fopen(status_path, "r")) == NULL) {
        return;
    }

    char* file_content;
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

    size_t bytes = fread(file_content, 1, FILE_BUFFER_SIZE - 1, file);
    file_content[bytes] = '\0';
    char* parsed = parse(file_content);
    if (parsed) {
        printf("PID %s: %s\n", pid, parsed);
        free(parsed);
    }
    free(file_content);
    if(fclose(file) != 0) {
        if (fprintf(stderr, "Eroare inchiderea fisierului\n") < 0) {
            perror("Eroare la scrierea in stderr\n");
        }
    }
}

int main() {
    struct dirent *entry;
    DIR* dir;

    regex_t regex;
    if (regcomp(&regex, "^[0-9]+$", REG_EXTENDED) != 0) {
        if (fprintf(stderr, "Eroare la compilare regex\n") < 0) {
            perror("Eroare la scrierea in stderr\n");
        }
        exit(1);
    }

    if ((dir = opendir("/proc")) == NULL) {
        if (fprintf(stderr, "Eroare la deschiderea directorului /proc\n") < 0) {
            perror("Eroare la scrierea in stderr\n");
        }
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (regexec(&regex, entry->d_name, 0, NULL, 0) != 0) {
            continue;
        }
        print_process(entry);
    }

    closedir(dir);
    regfree(&regex);

    return 0;
}

/*

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