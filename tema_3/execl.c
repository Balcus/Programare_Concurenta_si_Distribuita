#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 3: execl.c
 * Programul va folosi execl 
 * pentru a apela functia de system df cu argumentul -h
 * pentru a printa spatiul valabil de pe disk in format human readable
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
        execl("/bin/df", "df", "-h", NULL);
        perror("Eroare la apelul execl()");
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

% ./build/app4
Filesystem        Size    Used   Avail Capacity iused ifree %iused  Mounted on
/dev/disk3s1s1   228Gi    12Gi   103Gi    11%    455k  1.1G    0%   /
devfs            201Ki   201Ki     0Bi   100%     698     0  100%   /dev
/dev/disk3s6     228Gi    20Ki   103Gi     1%       0  1.1G    0%   /System/Volumes/VM
/dev/disk3s2     228Gi   8.1Gi   103Gi     8%    1.5k  1.1G    0%   /System/Volumes/Preboot
/dev/disk3s4     228Gi   6.0Mi   103Gi     1%     109  1.1G    0%   /System/Volumes/Update
/dev/disk2s2     500Mi   6.0Mi   481Mi     2%       1  4.9M    0%   /System/Volumes/xarts
/dev/disk2s1     500Mi   6.0Mi   481Mi     2%      28  4.9M    0%   /System/Volumes/iSCPreboot
/dev/disk2s3     500Mi   1.8Mi   481Mi     1%      96  4.9M    0%   /System/Volumes/Hardware
/dev/disk3s5     228Gi   104Gi   103Gi    51%    1.2M  1.1G    0%   /System/Volumes/Data
map auto_home      0Bi     0Bi     0Bi   100%       0     0     -   /System/Volumes/Data/home

*/