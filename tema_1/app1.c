/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 1: app1.c
 * Recreem comanda echo sub numele de yell :)
 * Comanda v-a functiona doar daca primeste de la utilizator dupa numele programului
 * (./yell) si un argument de tip string pe care sa il proceseze.
 * Comanda v-a a avea 2 optiuni care nu necesita valoari:
 * - (-h/--help) v-a afisa un ghid de folosire pentru comanda
 * - (-c/--capital) v-a transforma sirul de caractere in litere mari
 * si una care v-a necesita o valoare:
 * - (-o/--output) numele fisierului in care v-a fi scris output-ul comenzii. 
 * In cazul in care fisierul nu exista, acesta v-a fi creeat
 */

#include <stdio.h> // perror()
#include <stdlib.h> // exit(), abort()
#include <getopt.h> // getopt_long(), optarg, optind, structura option
#include <ctype.h> // to_upper()
#include <unistd.h> // write(), close()
#include <string.h> // strlen()
#include <fcntl.h> // open() si constantele O_WRONLY, O_CREAT, O_TRUNC

// functia help primeste un file descriptor si scrie in el mesajul de ajutor
void help(int fd) {
	char* message =	
		"Utilizare: ./yell [string] options [output-file]\n\n"
		"Optiuni:\n"
		"-h --help                   Afiseaza informatii referitoare la utilizarea comenzii\n"
		"-c --capital                Transforma caracterele sirului in CARACTERE MARI\n"
		"-o --output [output-file]   Scrie sirul de caractere dat in fisierul cu numele ales dupa prelucrare daca este cazul\n";

		// incercam sa scriem in file descriptor, daca nu merge, cum folosim syscalls afisam eroarea cu perror
		if(write(fd, message, strlen(message)) == -1) {
			perror("Eroare la scrierea in file descriptor");
			exit(1);
		}
}

char* fcapital(char* str) {
	// salvam referinta la primul caracter al sirului, vom avea nevoie de ea pentru a
	// returna sirul dupa ce l-am modificat
	char* res = str;

	// parcurgem pana ajungem la terminatorul null
	while (*str != '\0') {
		// transformam fiecare capracter in litera mare
		*str = toupper(*str);
		// incrementam pointerul pentru a ajunge la adresa de memorie a urmatorului caracter
		str++;
	}

	// returnam un pointer spre primul caracter din sir
	return res;
}

// exemplul pe care mi-am bazat logica de parsare a optiunilor folosind getopt_long este cel din Advanced Linux Programming
int main(int argc, char* argv[]) {
	// variabila in care se v-a stoca optiunea
	int next_option;
	// flag pentru a marca daca trebuie sa transformam string-ul in litere mari
	int capital = 0;
	// variabila in care vom stoca numele fisierului dedicat output
	char* output_filename = NULL;

	// sir de caractere pe care i-l vom da functiei getopt_long
	// simbolul : marcheaza faptul ca optiunea de dinainte cere o valoare
	const char* const short_options = "ho:c";

	// structura de care are nevoie functia getopt_long
	// format: nume_long | nr argumente | NULL | nume_short
	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"capital", 0, NULL, 'c'},
		{"output", 1, NULL, 'o'},
		{NULL, 0, NULL, 0}
	};

	// getopt_long returneaza fiecare optiune pe rand asa ca folosim functia in interiorul unui loop
	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		// folosim instructiunea switch pentru a decide ce sa facem in cazul fiecarei optiuni
		switch(next_option) {
			// pentru help afisam mesajul si iesim instant cu valoarea 0 pentru a semnala executia cu success
			case 'h':
				help(1);
				exit(0);
			// pentru output salvam numele fisierului si continuam
			// in momentul in care avem de a face cu optiuni care necesita argumente,
			// valoarea argumentului v-a fi stocata in optarg
			case 'o':
				output_filename = optarg;
				break;
			// pentru optiunea capital setam flag-ul pe 1 si continuam
			case 'c':
				capital = 1;
				break;
			// in cazul in care getopt_long returneaza ? inseamna ca optiunea este
			// invalida asa ca afisam mesajul de ajutor scris in stderr si iesim
			// cu codul 1 care sa semnalizeze eroarea
			case '?':
				help(2);
				exit(1);
			// in cazul in care getopt_long returneaza -1 inseamna ca optiunile s-au terminat
			// deci iesim din loop
			case -1:
				break;
			default:
				// folsoim abort pentru a semnala a anormalitate in executia programului
				// potrivit pentru o optiune neasteptata care ar ajunge pe ramura default
				abort();
		}
	}while(next_option != -1);

	// odata cu procesarea optiunilor, optind retine index-ul primului argument care nu este numele programului sau optiune
	// daca acesta este < nuamrul de argumente inseamna ca exista, daca nu afisam help si iesim cu 1
	if (optind >= argc) {
		help(2);
		exit(1);
	}

	// luam din argv string-ul introdus de utilizator
	char* str = argv[optind];
	// daca flag-ul de capital este setat transformam sirul
	if (capital == 1) {
		fcapital(str);
	}

	// daca nu este specificat un fisier de output scriem in stdout
	if (output_filename == NULL) {
		write(1, str, strlen(str));
		
	} else {
		const int file_permission = 06444;
		int fd;
		// daca este specificat un fisier de output incercam sa deschidem fisierul (sau creem si deschidem) si tratam exceptia daca apare
		if ((fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, file_permission)) == -1) {
			perror("Eroare la deschiderea fisierului");
			exit(1);
		}
		// incercam sa scriem in fisier string-ul dat de utilizator si trartam exceptia daca apare
		if(write(fd, str, strlen(str)) == -1) {
			perror("Eroare la scrierea in file descriptor");
			exit(1);
		}

		// inchidem fisierul
		close(fd);
	}

	return 0;
}

 /*
Script started on Thu Feb 26 17:05:27 2026

% clang-tidy app1.c -- -std=c17
47 warnings generated.
Suppressed 47 warnings (47 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.

% ./yell
Utilizare: ./yell [string] options [output-file]

Optiuni:
-h --help                   Afiseaza informatii referitoare la utilizarea programului
-c --capital                Transforma caracterele sirului in CARACTERE MARI
-o --output [output-file]   Scrie sirul de caractere dat in fisierul cu numele ales dupa prelucrare daca este cazul

% ./yell hi
hi

% ./yell -c hi
HI

% ./yell --capital hi
HI

% ./yell --capital hi -o app1_output.txt

% cat app1_output.txt
HI

% ./yell hi --output app1_output.txt
Eroare la deschiderea fisierului: Permission denied

% sudo ./yell hi --output app1_output.txt
Password:

% cat app1_output.txt
hi

% ./yell -h
Utilizare: ./yell [string] options [output-file]

Optiuni:
-h --help                   Afiseaza informatii referitoare la utilizarea programului
-c --capital                Transforma caracterele sirului in CARACTERE MARI
-o --output [output-file]   Scrie sirul de caractere dat in fisierul cu numele ales dupa prelucrare daca este cazul

% ./yell hi -c
HI

% ./yell -x
yell: invalid option -- x
Utilizare: ./yell [string] options [output-file]

Optiuni:
-h --help                   Afiseaza informatii referitoare la utilizarea programului
-c --capital                Transforma caracterele sirului in CARACTERE MARI
-o --output [output-file]   Scrie sirul de caractere dat in fisierul cu numele ales dupa prelucrare daca este cazul

% ./yell -capital hi
yell: invalid option -- a
Utilizare: ./yell [string] options [output-file]

Optiuni:
-h --help                   Afiseaza informatii referitoare la utilizarea programului
-c --capital                Transforma caracterele sirului in CARACTERE MARI
-o --output [output-file]   Scrie sirul de caractere dat in fisierul cu numele ales dupa prelucrare daca este cazul

% exit

Script done on Thu Feb 26 17:07:18 2026
*/