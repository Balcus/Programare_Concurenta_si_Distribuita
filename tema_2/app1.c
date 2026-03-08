#include <getopt.h> // getopt_long(), optarg, structura option
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int next_option;
    int processes = 0;

    const char* const short_options = "p:";
	const struct option long_options[] = {
		{"processes", 0, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};

    do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		switch(next_option) {
			case 'p':
                processes = atoi(optarg);
				break;
			case '?':
				exit(1);
			case -1:
				break;
			default:
				abort();
		}
	}while(next_option != -1);

    if (processes < 0) {
        if (fprintf(stderr, "Numarul de procese trebuie sa fie o valoare pozitiva\n") < 0) {
            perror("Eroare la scrierea in stderr");
        }
        exit(1);
    }

    printf("Process[A] PID %d PPID %d\n", (int)getpid(), (int)getppid());

    int status;
    pid_t pid_b = fork();
    if (pid_b == 0) {
        pid_t children[processes];
        printf("Process[B] PID %d PPID %d\n", (int)getpid(), (int)getppid());

        for (int i = 0 ; i < processes ; i++) {
            pid_t child_pid = fork();
            if (child_pid == 0) {
                printf("Process[%d] PID %d PPID %d\n", i, (int)getpid(), (int)getppid());
                exit(0);
            }else {
                children[i] = child_pid;
            }
        }

        for (int i = 0 ; i < processes ; i++) {
            int status;
            waitpid(children[i], &status, 0);
        }

        exit(0);
    }

    waitpid(pid_b, &status, 0);
    return 0;
}

/*

% clang-tidy app1.c -- -std=c17
41 warnings generated.
Suppressed 41 warnings (40 in non-user code, 1 with check filters).
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

*/