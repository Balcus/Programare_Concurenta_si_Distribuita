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
#include <time.h> // time(), avem nevoie pentru functia de seed
#include <unistd.h> // write(), close()
#include <stdio.h> // perror(), stdout, fprintf()
#include <stdlib.h> // exit(), abort(), srand(), rand(), atoi()
#include <getopt.h> // getopt_long_only(), optarg, optind, structura option

#define DEFAULT_FROM 0
#define DEFAULT_TO 100

// functia help primeste un file descriptor si scrie in el mesajul de ajutor
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

// functie pentru generarea de numere random intr-un anume interval
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
    // Facem seed la generatorul de numere random folosit de functia rand()
    srand(time(NULL));

    // variabila in care v-a fi stocata optiunea
    int next_option;

    // variabilele in care vor fi stocate valorile pentru optiuni
    int from = DEFAULT_FROM;
    int to = DEFAULT_TO;
    int even_only = 0;

    // sir de caractere de care are nevoie functia getopt_long_only()
	// 'h' si 'e' fara valoare (nu au ':' dupa ele), f,t cu valoare
    const char* const short_options = "hf:t:e";

    // structura de care are nevoie functia getopt_long_only()
	// format: nume_long | nr argumente | NULL | nume_short
    const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"from", 1, NULL, 'f'},
		{"to", 1, NULL, 't'},
        {"even-only", 0, NULL, 'e'},
		{NULL, 0, NULL, 0}
	};

    // parcurgem argumentele primte de la linia de comanda unul cate unul intr-o bucla
    do {
        // citim argumentul folosind getopt_long_only()
        next_option = getopt_long_only(argc, argv, short_options, long_options, NULL);
        switch (next_option) {
            // pentru help afisam mesajul si iesim cu exit code 0
            case 'h':
                help(1);
                exit(0);
            // pentru optiunea from citim valoarea din optarg si facem transformarea din ascii in int folosind atoi() apoi o stocam in variabila from
            case 'f':
                from = atoi(optarg);
                break;
            // pentru optiunea to facem acelasi lucru ca la from si stocam in variabila to
            case 't':
                to = atoi(optarg);
                break;
            // pentru optiunea even-only setam falg-ul de even_only pe true
            case 'e':
                even_only = 1;
                break;
            // in cazul in care intalnim o optiune invalida scriem mesajul de help la stderr si iesim cu exit code 1 pentru a semnala eroarea
            case '?':
                help(2);
                exit(1);
            // in cazul in care getopt_long_only returneaza -1 inseamna ca optiunile s-au terminat
            case -1:
                break;
            // pentru un caz neprevazut vom folosi abort() pentru a semnala a anormalitate in executia programului
            default:
                abort();

        }
    } while(next_option != -1);

    // computam numarul random si incercam sa il scriem la stdout folosind fprintf
    // in cazul in care fprintf() returneaza o eroare, o afisam folosind perror impreuna cu un mesaj descriptiv
    // putem folosi perror deoarece fprinf() seteaza errno in caz de eroare
    int res = vrand(from, to, even_only);
    if (fprintf(stdout, "%d\n", res) == -1) {
        perror("Erorare la scrierea in stdout");
    }

    return 0;
}

/*
Script started on Sat Feb 28 00:25:09 2026

% clang-tidy app2.c -- -std=c17
40 warnings generated.
Suppressed 40 warnings (40 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.

% ./vrand
7

% ./vrand -h
Utilizare: ./rand options

Optiuni:
-h --help          Afiseaza informatii referitoare la utilizarea programului
-f --from          Limita inferioara a range-ului de numere (DEFAULT = 0)
-t --to            Limita inferioara a range-ului de numere (DEFAULT = 100)
-e --even-only     Garanteaza ca numarul extras v-a fi par

% ./vrand -help
Utilizare: ./rand options

Optiuni:
-h --help          Afiseaza informatii referitoare la utilizarea programului
-f --from          Limita inferioara a range-ului de numere (DEFAULT = 0)
-t --to            Limita inferioara a range-ului de numere (DEFAULT = 100)
-e --even-only     Garanteaza ca numarul extras v-a fi par

% ./vrand -f 100
100

% ./vrand -f 100 -t 1000
519

% ./vrand -f 100 -t 1000
796

% ./vrand -f 100 -t 1000 -e
102

% ./vrand -f 100 -t 1000 -e
380

% ./vrand -f 100 -t 1000 -e
968

% ./vrand -f 100 -t 1000 -even-only
310

% ./vrand -f 100 -t 1000 -even-only
552

% ./vrand -f 100 -to 1000 --even-only
134

% ./vrand --from 100 -to 1000 --even-only
-60

% ./vrand --from -100 -to -10 --even-only
-82

% ./vrand -x
vrand: unrecognized option `-x'
Utilizare: ./rand options

Optiuni:
-h --help          Afiseaza informatii referitoare la utilizarea programului
-f --from          Limita inferioara a range-ului de numere (DEFAULT = 0)
-t --to            Limita inferioara a range-ului de numere (DEFAULT = 100)
-e --even-only     Garanteaza ca numarul extras v-a fi par

% exit

Script done on Sat Feb 28 00:27:06 2026
*/