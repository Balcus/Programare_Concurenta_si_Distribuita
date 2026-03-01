#include "argtable3.h" // arg_int_t, arg_lit_t, arg_file_t, arg_end_t, arg_lit0(), arg_int0(), arg_filen(), arg_end(), arg_nullcheck()
                       // arg_print_syntax(), arg_print_glossary(), arg_freetable()
#include <stdio.h> // printf(), fprintf(), perror(), fopen(), fgets(), fcloase()
#include <stdlib.h> // exit()

#define MAX_ERROR_NUMBER 20
#define MAX_NUMBER_OF_FILES 10
#define LINE_BUFFER_SIZE 1024

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 1: app3.c
 * Vom implementa comanda first conform cerintelor pentru task-ul bonus nivelului A:
 * Comanda v-a primi unul sau mai multe nume de fisere si va afisa peimele n linii
 * Optiuni:
 * (-h/--help) indicatii pentru folosirea comenzii
 * (-l/--lines) numerul de linii care v-a fi afisat din fiser numar intreg pozitiv, daca nu este specificata aceasta optiune afisam tot continutul
 * (-q/--quiet) nu afiseaza header-ul pentru fisier in output
 */

// functie care elibereaza memoria alocata pentru argtable inainte de a iesi din program cu exit code-ul dat
// primeste ca parametrii exitcode-ul programului, un pointer spre argtable cat si capacitatea de memorie care trebuie dealocata
// un simplu pointer nu ar fi destul deoarece: https://www.geeksforgeeks.org/cpp/array-decay-in-c/
 void free_argtable_and_exit(int exitcode, void *argtable[], int n) {
  arg_freetable(argtable, n);
  exit(exitcode);
}

int main(int argc, char *argv[]) {
  // variabila in care stocam exit code-ul programului
  int exitcode = 0;

  // variabila in care stocam numarul de linii pe care sa il afisam
  arg_int_t *lines;
  // variabile pentru optiunile care nu necesita valori (flag-uri)
  arg_lit_t *quiet, *help;
  // variabila in care stocam informatii despre fisierele de intrare
  arg_file_t *files;
  // variabila folosita pentru colectarea erorilor si care marcheaza sfarsitul argtable
  arg_end_t *end;

  void *argtable[] = {
    help = arg_lit0("h", "help", "Afiseaza informatii referitoare la utilizarea comenzii"),
    quiet = arg_lit0("q", "quiet", "Opreste afisarea de headere inainte de continutul fisierelor"),
    lines = arg_int0("l", "lines", "<int>", "Numarul de linii care sa fie afisat"),
    files = arg_filen(NULL, NULL, "<file_names>", 1, MAX_NUMBER_OF_FILES, "Numele fisierelor de intrare"),
    end = arg_end(MAX_ERROR_NUMBER)
  };

  // verificam ca argtable sa fie alocat corect
  if (arg_nullcheck(argtable) != 0) {
    // in cazul in care fprintf() returneaza o eroare, o afisam cu perror
    if(fprintf(stderr, "Eroare la alocarea de memorie pentru argtable\n") < 0) {
      perror("Eroare la scrierea in stderr");
    }
    return 1;
  }

  // argparse va parsa argumentele date si va returna numarul de erori gasite pe care il stocam in variabila nerrors
  int nerrors = arg_parse(argc, argv, argtable);

  // salvam numele programului pentru mesajul de help
  char* progname = argv[0];

  // salvam numarul de elemente ale argtable deoarece funtia arg_freetable() are nevoie de numarul de elemente din argtable
  int n_elem = sizeof(argtable) / sizeof(argtable[0]);

  // daca am gasit optiunea de help afisam mesajul si iesim cu exit code 0 folosind functia free_argtable_and_exit
  // pentru a elibera memoria folosita de argtable
  if (help->count > 0) {
    printf("Usage: %s", progname);
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    free_argtable_and_exit(exitcode, argtable, n_elem);
  }

  // daca am intampinat erori de parsare atunci setam exitcode-ul pe 1, afisam un mesaj de utilizare al programului
  // si iesim folosind functia free_argtable_and_exit pentru a elibera memoria folosita de argtable
  if (nerrors > 0) {
    exitcode = 1;
    arg_print_errors(stderr, end, argv[0]);
    printf("Usage: %s", progname);
    arg_print_syntax(stderr, argtable, "\n");
    free_argtable_and_exit(exitcode, argtable, n_elem);
  }

  // daca am primit optiunea lines cu o valoare <= 0 afisam eroarea la stderr, setam exitcode la 1, eliberam memoria pentru argtable si iesim
  if (lines->count > 0 && *lines->ival <= 0) {
    exitcode = 1;
    if ((fprintf(stderr, "Valoarea optiunii lines trebuie sa fie un numar pozitiv mai mare ca 0\n") < 0)) {
      perror("Eroare la scrierea in stderr");
    }
    free_argtable_and_exit(exitcode, argtable, n_elem);
  }

  // buffer pentru stocarea unei linii
  char line[LINE_BUFFER_SIZE];

  // in cazul in care am primit fisiere de intrare
  if (files->count > 0) {
    // parcurgem lista de fisiere
    for(int i = 0 ; i < files->count ; i++) {
      // incercam sa deschidem fiecare fisier pe rand
      FILE* fptr;
      // in caz de eroare la deschiderea fisierului setam exitcode la 1, printam eroarea si trecem la urmatorul fisier
      if ((fptr = fopen(files->filename[i], "r")) == NULL) {
        exitcode = 1;
        perror("Eroare la deschidere de fiser");
        continue;
      }

      // daca nu am primit falg-ul de quiet atunci afisam header-ul pentru fiecare fisier
      if (quiet->count == 0) {
        printf("\n--- %s ---\n", files->filename[i]);
      }

      // daca nu s-a dat optiunea de lines citim tot fisierul si il printam in stdout
      if (lines->count == 0) {
        while (fgets(line, sizeof(line), fptr)) {
          printf("%s", line);
        }
        // incercam sa inchidem fisierul, in caz de eroare setam exitcode la 1 si afisam mesajul de eroare
        if (fclose(fptr)) {
          exitcode = 1;
          perror("Eroare la inchiderea fisierului");
        }
        // trecem la urmatorul fisier
        continue;
      }

      // in cazul in care primim o valoare corecta pentru argumentul lines, citim primele n linii si le afisam la stdout
      int val = *lines->ival;
      // variabila in care retinem numarul de linii pe care le-am citit
      int c = 0;
      while (fgets(line, sizeof(line), fptr) && c < val) {
        printf("%s", line);
        c++;
      }
      // incercam sa inchidem fisierul, in caz de eroare setam exitcode la 1 si afisam eroarea cu perror
      if (fclose(fptr)) {
        exitcode = 1;
        perror("Eroare la inchiderea fisierului");
      }
    }
  }
  
  // la finaulul programului eliberam memoria pentru argtable si returnam exitcode
  free_argtable_and_exit(exitcode, argtable, n_elem);
}

/*
Script started on Sun Mar  1 14:55:14 2026

% ./first
./first: missing option <file_names>
[-hq] [-l <int>] <file_names> [<file_names>]...
Usage: ./first

% ./first -h
Usage: ./first [-hq] [-l <int>] <file_names> [<file_names>]...
-h, --help                Afiseaza informatii referitoare la utilizarea comenzii
-q, --quiet               Opreste afisarea de headere inainte de continutul fisierelor
-l, --lines=<int>         Numarul de linii care sa fie afisat
<file_names>              Numele fisierelor de intrare

% ./first --help
Usage: ./first [-hq] [-l <int>] <file_names> [<file_names>]...
-h, --help                Afiseaza informatii referitoare la utilizarea comenzii
-q, --quiet               Opreste afisarea de headere inainte de continutul fisierelor
-l, --lines=<int>         Numarul de linii care sa fie afisat
<file_names>              Numele fisierelor de intrare

% cat numbers.txt
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15

% cat chars.txt
a
b
c
d
e
f
g
h
i

% ./first numbers.txt -1
./first: invalid option "-1"
[-hq] [-l <int>] <file_names> [<file_names>]...
Usage: ./first

% ./first numbers.txt -l -1
Valoarea optiunii lines trebuie sa fie un numar pozitiv mai mare ca 0

% ./first numbers.txt -l 1

--- numbers.txt ---
1

% ./first numbers.txt -l 10

--- numbers.txt ---
1
2
3
4
5
6
7
8
9
10

% ./first numbers.txt -l 100

--- numbers.txt ---
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15

% ./first numbers.txt -l 5 -q
1
2
3
4
5

% ./first numbers.txt chars.txt --lines 5

--- numbers.txt ---
1
2
3
4
5

--- chars.txt ---
a
b
c
d
e

% ./first numbers.txt chars.txt --lines 5 --quiet
1
2
3
4
5
a
b
c
d
e

% exit

Script done on Sun Mar  1 14:56:58 2026
*/