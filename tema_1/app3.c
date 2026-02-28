#include "argtable3.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_ERROR_NUMBER 20
#define MAX_NUMBER_OF_FILES 10

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 1: app3.c
 * vom implementa comanda first conform cerintelor pentru task-ul bonus nivelului A:
 * comanda v-a primi unul sau mai multe nume de fisere si v-a afisa peimele n linii
 * Optiuni:
 * (-h/--help) indicatii pentru folosirea comenzii
 * (-l/--lines) numerul de linii care v-a fi afisat din fiser numar intreg pozitiv, daca nu este specificata aceasta optiune afisam tot continutul
 * (-q/--quiet) nu afiseaza header-ul pentru fisier in output
 */

 arg_int_t *lines;
 arg_lit_t *quiet, *help;
 arg_file_t *files;
 arg_end_t *end;

 void exit_first(int exitcode, void *argtable[], int n) {
  arg_freetable(argtable, n);
  exit(exitcode);
}

int main(int argc, char *argv[]) {
  void *argtable[] = {
    help = arg_lit0("h", "help", "Afiseaza informatii referitoare la utilizarea comenzii"),
    quiet = arg_lit0("q", "quiet", "Opreste afisarea de headere inainte de continutul fisierelor"),
    lines = arg_int0("l", "lines", "<int>", "Numarul de linii care sa fie afisat"),
    files = arg_filen(NULL, NULL, "<file_names>", 1, MAX_NUMBER_OF_FILES, "Numele fisierelor de intrare"),
    end = arg_end(MAX_ERROR_NUMBER)
  };

  if (arg_nullcheck(argtable) != 0) {
    if(fprintf(stderr, "Eroare la alocarea de memorie pentru argtable\n") == -1) {
      perror("Eroare la scrierea in stderr");
    }
    return 1;
  }

  int nerrors = arg_parse(argc, argv, argtable);
  char* progname = argv[0];
  int nelem = sizeof(argtable) / sizeof(argtable[0]);

  if (help->count > 0) {
    printf("Usage: %s", progname);
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    exit_first(0, argtable, nelem);
  }

  if (nerrors > 0) {
    arg_print_errors(stderr, end, argv[0]);
    printf("Usage: %s", progname);
    arg_print_syntax(stderr, argtable, "\n");
    exit_first(1, argtable, nelem);
  }
  
  exit_first(0, argtable, nelem);
}
