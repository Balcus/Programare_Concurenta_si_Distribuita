#include <stdio.h>
#include <stdlib.h>
#include <getopt.h> // functia get_opt_long()
#include <ctype.h> // functia to_upper
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/*
voi incerca sa recreez comanda echo sub numele de yell :)
comanda v-a functiona doar daca primeste de la utilizator dupa numele programului
(./yell) si un argument de tip sir de caractere pe care sa il proceseze
comanda v-a a avea 2 optiuni care nu necesita valoari:
- (-h/--help) v-a afisa un ghid de folosire pentru comanda
- (-c/--capital) v-a transforma sirul de caractere in litere mari
si una care v-a necesita o valoare:
- (-o/--output) valoarea pe care o primim ar trebui sa fie numele unui fisier in
care utilitara sa scrie valoarea primita. In cazul in care fisierul nu exista, acesta
v-a fi creeat
*/

// functia help v-a primi un file descriptor si v-a scrie in acesta mesaju de ajutor
// 1 -> stdout
// 2 -> stderr
void help(int fd) {
	char* message =	
		"Utilizare: ./yell [string] options [output-file]\n\n"
		"Optiuni:\n"
		"-h --help                   Afiseaza informatii referitoare la utilizarea programului\n"
		"-c --capital                Transforma caracterele sirului in CARACTERE MARI\n"
		"-o --output [output-file]   Scrie sirul de caractere dat in fisierul cu numele ales dupa prelucrare daca este cazul\n";

		// incercam sa scriem in file descriptor iar daca acest lucru nu merge,
		// cum folosim syscalls, putem folosi perror
		if(write(fd, message, strlen(message)) == -1) {
			perror("Eroare la scrierea in file descriptor");
			exit(1);
		}
}

char* fcapital(char* str) {
	// salvam referinta la primul caracter al sirului, vom avea de ea pentru a
	// returna sirul dupa ce l-am modificat
	char* res = str;

	// dereferentiem adresa de memorie si parcurgem pana la acea adreasa gasim 
	while (*str != '\0') {
		// inlocuim valoarea cu rezultatul de la toupper pentru valoarea
		*str = toupper(*str);
		// incrementam pointerul pentru a ajunge la urmatoarea adresa de memorie pe
		// care urmeaza sa o verificam
		str++;
	}

	// returnam adresa de memorie de la care incepe sirul de caractere pe
	// care am salvat-o la inceput (nu putem sa returnam str deoarece a ramas la sf.
	// sirului de caractere)
	return res;
}

// exemplul pe care mi-am bazat logica de parsare a optiunilor folosind getopt_long
// este cel din Advanced Linux Programming
int main(int argc, char* argv[]) {
	// variabila in care vom stoca optiunile
	int next_option;
	// flagul de capital, initializat cu 0 (false)
	int capital = 0;
	// variabila in care vom stoca numele fisierului dedicat output
	char* output_filename = NULL;

	// sir de caractere pe care i-l vom da functiei getopt_long, simbolul :
	// marcheaza faptul ca optiunea de dinainte cere o valoare
	const char* const short_options = "ho:c";

	// structura necesare pe care i-o vom da functiei getopt_long
	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"capital", 0, NULL, 'c'},
		{"output", 1, NULL, 'o'},
		{NULL, 0, NULL, 0}
	};

	// getopt_long returneaza cate o singura optiune pe rand asa ca folosim functia in
	// interiorul instructiunii de tip do while, citind optiunuile pana in momentul
	// in care se termina
	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		// folosim instructiunea switch pentru a decide ce sa facem in cazul
		// fiecarei optiuni
		switch(next_option) {
			// pentru help afisam mesajul si iesim instant cu valoarea 0
			// pentru a semnala executia cu success, ignorand restul optiunilor
			case 'h':
				help(1);
				exit(0);
			// pentru optiunea de output salvam numele fisierului si continuam executia
			// in momentul in care avem de a face cu optiuni care necesita argumente,
			// valoarea argumentului v-a fi stocata in optarg
			case 'o':
				output_filename = optarg;
				break;
			// pentru optiunea capital doar setam un flag pe 1 si continuam
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
			// deci iesim din bucla
			case -1:
				break;
			default:
				abort();
		}
	}while(next_option != -1);

	// odata cu procesarea optiunilor, optind retine index-ul primului argument
	// care nu este numele programului sau optiune
	// verificam ca acesta sa nu fie mai mare decat numarul argumentelor,
	// adica practic, sa nu existe iar in aces caz, in care nu exista, afisam help
	// si iesim cu 1
	if (optind >= argc) {
		help(2);
		exit(1);
	}

	// luam din argv argumentul sir de caractere introdus de utilizator
	char* str = argv[optind];
	if (capital == 1) {
		fcapital(str);
	}

	if (output_filename == NULL) {
		write(1, str, strlen(str));
	} else {
		const int file_permission = 06444;
		int fd;
		if ((fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, file_permission)) == -1) {
			perror("Eroare la deschiderea fisierului");
			exit(1);
		}
		if(write(fd, str, strlen(str)) == -1) {
			perror("Eroare la scrierea in file descriptor");
			exit(1);
		}
		close(fd);
	}
}
