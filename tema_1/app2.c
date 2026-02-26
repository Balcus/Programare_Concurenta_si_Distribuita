/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 1: app2.c
 * Vom implementa o comanda numita vrand (very-random) pentru generarea de numere random
 * Optiuni:
 * (-f/--from) limita inferioara a range-ului de numere din care se v-a face alegerea, necesita o valoare, default 0
 * (-t/--to) limita superioara a range-ului de numere din care se v-a face alegerea, necesita o valoare, default 100
 * (-e/--even-only) numarul extras v-a fi par garantat, nu necesita valoare
 * (-h/--help) indicatii pentru folosirea comenzii
 */

#include <string.h> // strlen()
#include <time.h> // time()
#include <unistd.h> // write(), close()
#include <stdio.h> // perror(), stdout
#include <stdlib.h> // exit(), abort()
#include <getopt.h> // getopt_long(), optarg, optind, structura option

#define DEFAULT_FROM 0
#define DEFAULT_TO 100

// functia help primeste un file descriptor si scrie in el mesajul de help
void help(int fd) {
	char* message =	
		"Utilizare: ./rand options\n\n"
		"Optiuni:\n"
		"-h --help          Afiseaza informatii referitoare la utilizarea programului\n"
		"-f --from          Limita inferioara a range-ului de numere (DEFAULT = 0)\n"
		"-t --to            Limita inferioara a range-ului de numere (DEFAULT = 100)\n"
        "-e --even-only     Garanteaza ca numarul extras v-a fi par\n";

		// incercam sa scriem in file descriptor, daca nu merge, cum folosim syscalls afisam eroarea folosind perror cu un mesaj descriptiv
		if(write(fd, message, strlen(message)) == -1) {
			perror("Eroare la scrierea in file descriptor");
			exit(1);
		}
}

int vrand(int f, int t, int e) {
    // https://stackoverflow.com/questions/1202687/how-do-i-get-a-specific-range-of-numbers-from-rand
    int res = rand() % (t + 1 - f) + f;
    if (e != 0) {
        if (res % 2 != 0) {
            if (res + 1 > t) {
                res = res - 1;
            }else {
                res = res + 1;
            }
        }
    }
    return res;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    int next_option;

    int from = DEFAULT_FROM;
    int to = DEFAULT_TO;
    int even_only = 0;

    const char* const short_options = "hf:t:e";

    const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"from", 1, NULL, 'f'},
		{"to", 1, NULL, 't'},
        {"even-only", 0, NULL, 'e'},
		{NULL, 0, NULL, 0}
	};

    do {
        next_option = getopt_long_only(argc, argv, short_options, long_options, NULL);
        switch (next_option) {
            case 'h':
                help(1);
                exit(0);
            case 'f':
                from = atoi(optarg);
                break;
            case 't':
                to = atoi(optarg);
                break;
            case 'e':
                even_only = 1;
                break;
            case '?':
                help(2);
                exit(1);
            case -1:
                break;
            default:
                abort();

        }
    }while(next_option != -1);

    int res = vrand(from, to, even_only);
    if (fprintf(stdout, "%d\n", res) == -1) {
        perror("Erorare la scrierea in stdout");
    }

    return 0;
}
