#include <getopt.h> // getopt_long(), optarg, structura option
#include <stdio.h>
#include <stdlib.h>

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 2: app1.c
 * Programul v-a realiza structura de procese ceruta la cerinta 1 si respectiv 2
*/

//TODO: add -1 check for fork()

int main(int argc, char* argv[]) {
    int next_option, processes = 0, subprocesses = 0;

    const char* const short_options = "p:s:";
	const struct option long_options[] = {
		{"processes", 1, NULL, 'p'},
        {"subprocesses", 1, NULL, 's'},
		{NULL, 0, NULL, 0}
	};

    do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		switch(next_option) {
			case 'p':
                processes = atoi(optarg);
				break;
            case 's':
                subprocesses = atoi(optarg);
				break;
			case '?':
				exit(1);
			case -1:
				break;
			default:
				abort();
		}
	}while(next_option != -1);

    if (processes < 0 || subprocesses < 0) {
        if (fprintf(stderr, "Numarul de procese/subprocese trebuie sa aibe o valoare pozitiva\n") < 0) {
            perror("Eroare la scrierea in stderr");
        }
        exit(1);
    }

    printf("Process[A] PID %d PPID %d\n", (int)getpid(), (int)getppid());

    pid_t pid_b = fork();
    if (pid_b == 0) {
        printf("Process[B] PID %d PPID %d\n", (int)getpid(), (int)getppid());
        pid_t pid_0 = fork();
        if (pid_0 == 0) {
            printf("Process[0] PID %d PPID %d\n", (int)getpid(), (int)getppid());
            for (int i = 1 ; i <= processes ; i++) {
                pid_t child_pid = fork();
                if (child_pid == 0) {
                    printf("Process[%d] PID %d PPID %d\n", i, (int)getpid(), (int)getppid());
                    for (int j = 1 ; j <= subprocesses ; j++) {
                        pid_t sbp_pid = fork();
                        if (sbp_pid == 0) {
                            printf("Process[%d.%d] PID %d PPID %d\n", i, j, (int)getpid(), (int)getppid());
                        }else {
                            wait(NULL);
                            exit(0);
                        }
                    }
                    exit(0);
                }
            }

            for (int i = 0 ; i < processes ; i++) {
                wait(NULL);
            }
            exit(0);
        }

        wait(NULL);
        exit(0);
    }

    wait(NULL);
    return 0;
}

/*

% clang-tidy app1.c -- -std=c17
40 warnings generated.
Suppressed 40 warnings (40 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.

% ./build/app1 -p 10
Process[A] PID 57742 PPID 56913
Process[B] PID 57743 PPID 57742
Process[0] PID 57744 PPID 57743
Process[1] PID 57745 PPID 57743
Process[2] PID 57746 PPID 57743
Process[3] PID 57747 PPID 57743
Process[4] PID 57748 PPID 57743
Process[5] PID 57749 PPID 57743
Process[6] PID 57750 PPID 57743
Process[7] PID 57751 PPID 57743
Process[8] PID 57752 PPID 57743
Process[9] PID 57753 PPID 57743

% ./build/app1 -p 2
Process[A] PID 57844 PPID 56913
Process[B] PID 57845 PPID 57844
Process[0] PID 57846 PPID 57845
Process[1] PID 57847 PPID 57845

% ./build/app1 -p -1
Numarul de procese trebuie sa fie o valoare pozitiva

./build/app1 -p 5 -s 3
Process[A] PID 2278 PPID 1160
Process[B] PID 2279 PPID 2278
Process[0] PID 2280 PPID 2279
Process[1] PID 2281 PPID 2280
Process[2] PID 2282 PPID 2280
Process[3] PID 2284 PPID 2280
Process[1.1] PID 2283 PPID 2281
Process[2.1] PID 2285 PPID 2282
Process[3.1] PID 2287 PPID 2284
Process[4] PID 2286 PPID 2280
Process[5] PID 2288 PPID 2280
Process[1.2] PID 2289 PPID 2283
Process[3.2] PID 2292 PPID 2287
Process[2.2] PID 2290 PPID 2285
Process[5.1] PID 2293 PPID 2288
Process[1.3] PID 2294 PPID 2289
Process[4.1] PID 2291 PPID 2286
Process[3.3] PID 2295 PPID 2292
Process[2.3] PID 2296 PPID 2290
Process[4.2] PID 2298 PPID 2291
Process[5.2] PID 2297 PPID 2293
Process[4.3] PID 2299 PPID 2298
Process[5.3] PID 2300 PPID 2297

*/