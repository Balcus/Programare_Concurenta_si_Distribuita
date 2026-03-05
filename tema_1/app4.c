#include "libconfig.h" // config_t, config_setting_t, config_init(), config_read_file(), config_error_file(), 
                       // config_error_line(), config_error_text(), config_destroy(), config_lookup_string(), 
                       // config_lookup(), config_setting_length(), config_setting_get_elem(), config_setting_lookup_string()
#include <getopt.h>    // struct option, getopt_long_only(), optarg
#include <stdio.h>     // printf(), fprintf(), perror(), snprintf(), stderr
#include <string.h>    // strcmp(), strlen()
#include <stdlib.h>    // setenv(), getenv(), exit(), atoi(), abort()
#include <unistd.h>    // write()

#define MAX_ENVIRON_VALUES_DEFAULT 10
#define MAX_ALLOWED_OP_DEFAULT 10
#define BUFFER_SIZE 256

/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 1: app3.c
 * Am rezolvat cerinta 5 intr-o noua aplicatie C deoarece nu mi-a venit o idee buna de
 * extindere a aplicatiei 3 astfel incat sa respecte cerintele si formatul fisierului cfg din tema
 * Programul v-a avea numele setenv si v-a avea ca rol setarea, afisarea si formatarea variabileleor de mediu
 * Programul va citi fisierul de configuratie si va fi capabil ca:
 * - in funcite de variabila active_profile sa aleaga profilul corect
 * - citeasca lista de operatii si in funcite de tipul operatiei sa seteze, formateze sau printeze o variabila de mediu
 * -a -–allowed-operations: numarul maxim de operatii pe care le putem avea intr-un fisier
 * Optiuni:
 * -h --help: afiseaza mesajul de ajutor
 * -m --max-environ-values: numarul maxim de variabile de mediu pe care le putem seta
 * exemplul folosit de mine va genera connection string-ul pentru o baza de date
 * postgres in functie de host, port, user, password
 */

 // functia help primeste un file descriptor si scrie in el mesajul de ajutor
void help(int fd) {
	char* message =	
		"Utilizare: ./setenv options\n\n"
		"Optiuni:\n"
		"-h --help                  Afiseaza informatii referitoare la utilizarea comenzii\n"
		"-m --max-environ-values    Numarul de variabile de mediu maxim\n"
        "-a -–allowed-operations    Numar de operatii permise";

		// incercam sa scriem in file descriptor, daca nu merge, cum folosim syscalls afisam eroarea folosind perror cu un mesaj descriptiv
		if(write(fd, message, strlen(message)) == -1) {
			perror("Eroare la scrierea in file descriptor");
			exit(1);
		}
}

 int main(int argc, char* argv[]) {
    // variabila pentru a retine exitcode-ul pe care sa il returneze aplicatia
    int exitcode = 0;
    // structura in care stocam configuratia aplicatiei
    config_t cfg;
    // strucutra in care vom stoca optiunile configuratiei
    config_setting_t *settings;
    // variabila in care vom stoca campul name din configuratie
    const char *name;
    // variabila in care vom stoca campul active_profile din configuratie
    const char *profile;
    // numele fisierului de configuratie
    const char* config_filename = "vars.cfg";
    // variabila in care v-a fi stocata optiunea
    int next_option;
    // initializam variabila care v-a stoca numarul maxim de var de mediu cu valoarea default
    int max_env_vars = MAX_ENVIRON_VALUES_DEFAULT;
    // initializam variabila care v-a stoca numarul maxim de operatii cu valoarea default
    int allowed_op = MAX_ALLOWED_OP_DEFAULT;

    // sir de caractere de care are nevoie functia getopt_long_only()
	// 'h' fara valoare, 'm' si 'a' cu valoare
    const char* const short_options = "hm:a:";

    // structura de care are nevoie functia getopt_long_only()
	// format: nume_long | nr argumente | NULL | nume_short
    const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"max-environ-values", 1, NULL, 'm'},
		{"allowed-operations", 1, NULL, 'a'},
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
            // setam max_env_vars la valoarea data de utilizator
            case 'm':
                max_env_vars = atoi(optarg);
                break;
            // setam allowed_op la valoarea data de utilizator
            case 'a':
                allowed_op = atoi(optarg);
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

    config_init(&cfg);

    // citim continutul fisierului de configuratie vars.cfg in variabila cfg
    if(!config_read_file(&cfg, config_filename)) {
        // in cazul in care intampinam o eroare la citirea configuratiei, o afisam, eliberam resursele folosite de structura cfg si returnam exitcode 1
        if (fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg)) < 0) {
            perror("Eroare la scrierea in stderr");
        }
        config_destroy(&cfg);
        return 1;
    }

    // cautam campul cu numele 'name' in configuratie,
    // daca il gasim afisam un mesaj care sa il contina,
    // in caz contrar trecem la urmatorul field si setam exitcode la 1
    if(config_lookup_string(&cfg, "name", &name)) {
        printf("Incepem configurarea folosind %s\n", name);
    }else {
        if(fprintf(stderr, "Variabila cu numele 'name' nu exista in fiserul: %s\n", config_filename) < 0) {
            perror("Eroare la scrierea in stderr");
        }
        exitcode = 1;
    }

    // cautam campul cu numele 'active_profile' in configuratie,
    // daca il gasim afisam un mesaj care sa il contina,
    // in caz contrar nu putem continua asa ca afisam mesajul de eroare, eliberam resursele folosite de cfg si returnam exit code 1
    if(config_lookup_string(&cfg, "active_profile", &profile)) {
        printf("Profil folosit: %s\n", profile);
    }else {
        if(fprintf(stderr, "Variabila cu numele 'profile' nu exista in fiserul: %s\n", config_filename) < 0) {
            perror("Eroare la scrierea in stderr");
        }
        config_destroy(&cfg);
        return 1;
    }

    // variabila in care stocam rezultatul returnat de snprintf pentru error checking
    int cx;
    // buffer in care stocam calea spre sectiunea de operatii din configurare
    char settings_path[BUFFER_SIZE];
    // formam un string care sa reprezinte sectiunea de operatii pentru profilul curent
    cx = snprintf(settings_path, BUFFER_SIZE, "profiles.%s.operations", profile);
    // in cazul in care apare o eroare la formatarea cu snprintf, afisam mesajul de eroare,
    // eliberam resursele folosite de cfg si returnam exitcode 1
    if (cx < 0 || cx >= BUFFER_SIZE) {
        if (fprintf(stderr, "Eroare la formatarea string-ului si stocarea in buffer\n") < 0) {
            perror("Eroare la scrierea in stderr");
        }
        config_destroy(&cfg);
        return 1;
    }

    // retinem in variabila settings sectiunea cu operatiile corespunzatoare pentru profilul curent
    settings = config_lookup(&cfg, settings_path);
    // in cazul in care aceasta sectiune nu exista afisam un mesaj de eroare, eliberam resursele si iesim cu exitcode 1
    if (settings == NULL) {
        if (fprintf(stderr, "Sectiuena cu nume %s nu se gaseste in fisierul %s\n", settings_path, config_filename) < 0) {
            perror("Eroare la scrierea in stderr");
        }
        config_destroy(&cfg);
        return 1;
    }

    // variabila care retine numarul de operatii din sectiune
    int op_count = config_setting_length(settings);
    // in cazul in care am depasit numarul admis de operatii ne vom opri, afisam mesajul de eroare, eliberam resursele folosite de cfg si returnam exit code 1
    if (op_count > allowed_op) {
        if (fprintf(stderr, "Numarul de operatii din fisier depaseste limita admisa. Limita:: %d, operatii: %d\n", allowed_op, op_count) < 0) {
            perror("Eroare la scrierea in stderr");
        }
        config_destroy(&cfg);
        return 1;
    }

    int env_count = 0;
    for (int i = 0; i < op_count; i++) {
        // variabila operation stocheaza operatia la care am ajuns din configuratie
        config_setting_t *operation = config_setting_get_elem(settings, i);
        // campurile prezente in operatie
        const char *action, *key, *value;

        // in cazul in care un camp nu este prezent, afisam eroarea, setam exitcode la 1 si continuam la urmatoarea operatie
        if (!(config_setting_lookup_string(operation, "action", &action)
            && config_setting_lookup_string(operation, "key", &key)
            && config_setting_lookup_string(operation, "value", &value)
        )) {
            if (fprintf(stderr, "Lipsesc campuri din structura operatiei\n") < 0) {
                perror("Eroare la scrierea in stderr");
            }
            exitcode = 1;
            continue;
        }

        // daca avem o operatie de add
        if (strcmp(action, "add") == 0) {
            // verificam daca limita este depasita, in acest caz afisam mesajul de eroare, setam exitcode la 1 si continuam
            if (env_count >= max_env_vars) {
                if (fprintf(stderr, "Numarul de variabile de mediu depaseste limita admisa. Limita: %d, variabile setate: %d\n", max_env_vars, env_count) < 0) {
                    perror("Eroare la scrierea in stderr");
                }
                exitcode = 1;
                continue;
            }
            // in cazul in care nu am depasit limita setam variabila de mediu si continuam
            setenv(key, value, 1);
            env_count++;
            continue;
        }

        // pentru operatia de format vom avea envoie de variabilele de mediu care au legatura cu baza de date setate
        if (strcmp(action, "format") == 0) {
            char* host = getenv("DB_HOST");
            char* port = getenv("DB_PORT");
            char* user = getenv("DB_USER");
            char* pass = getenv("DB_PASS");

            // in cazul in care nu sunt setate afisam un mesaj de eroare, setam exitcode la 1 si continuam la urmatoarea operatie
            if (host == NULL || port == NULL || user == NULL || pass == NULL) {
                if (fprintf(stderr, "Variabilele de mediu necesare nu au fost setate corect\n") < 0) {
                    perror("Eroare la scrierea in stderr");
                }
                exitcode = 1;
                continue;
            }

            // daca toate variabile de mediu sunt setate, folosim specificatorul de format din configuratie pentru a creea connection string-ul
            char res[BUFFER_SIZE];
            int cx;
            // in cazul in care apare o eroare la formatare afisam eroarea, setam exitcode la 1 si continuam la urmatoarea operatie
            cx = snprintf(res, BUFFER_SIZE, value, user, pass, host, port);
            if (cx < 0 || cx >= BUFFER_SIZE) {
                if (fprintf(stderr, "Eroare la formatarea string-ului si stocarea in buffer\n") < 0) {
                    perror("Eroare la scrierea in stderr");
                }
                exitcode = 1;
                continue;
            }
            // in cazul in care formatarea a functionat, setam variabila de mediu pentru connection string si continuam la urmatoarea optiune
            setenv(key, res, 1);
            continue;
        }

        // in cazul unei operatii de print incercam sa citim variabila de mediu
        if (strcmp(action, "print") == 0) {
            char* res = getenv(key);
            // in cazul in care aceasta nu exista afisam un mesaj de eroare, setam exitcode la 1 si continuam la urmatoarea operatie
            if (res == NULL) {
                if (fprintf(stderr, "Variabila de mediu %s nu a fost setata\n", key) < 0) {
                    perror("Eroare la scrierea in stderr");
                }
                exitcode = 1;
                continue;
            }
            // in cazul in care am gasit variabila de mediu, o afisam
            printf("%s: '%s'\n", key, res);
        }
    }

    // in final, eliberam resursele folosite de cfg si returnam exitcode-ul
    config_destroy(&cfg);
    return exitcode;
 }

 /*
Script started on Sun Mar  1 17:26:53 2026

% clang-tidy setenv.c -- -std=c17
40 warnings generated.
Suppressed 40 warnings (40 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.

% cat vars.cfg
name = "DbConfig";
version = 1.0;
active_profile = "staging";

profiles = {
    staging = {
        operations = (
            { action = "add";    
              key = "DB_HOST";    
              value = "localhost"; 
            },
            { action = "add";
              key = "DB_PORT";    
              value = "5433"; 
            },
            { action = "add";    
              key = "DB_USER";    
              value = "postgres"; 
            },
            { action = "add";    
              key = "DB_PASS";    
              value = "admin"; 
            },
            { 
              action = "format"; 
              key    = "DB_CONNECTION_STRING"; 
              value  = "postgres://%s:%s@%s:%s"; 
            },
            {
              action = "print";
              key = "DB_CONNECTION_STRING";
              value = "";
            },
        );
    };
};

% ./setenv
Incepem configurarea folosind DbConfig
Profil folosit: staging
DB_CONNECTION_STRING: 'postgres://postgres:admin@localhost:5433'

% ./setenv -m 1
Incepem configurarea folosind DbConfig
Profil folosit: staging
Numarul de variabile de mediu depaseste limita admisa. Limita: 1, variabile setate: 1
Numarul de variabile de mediu depaseste limita admisa. Limita: 1, variabile setate: 1
Numarul de variabile de mediu depaseste limita admisa. Limita: 1, variabile setate: 1
Variabilele de mediu necesare nu au fost setate corect

% ./setenv -a 2
Incepem configurarea folosind DbConfig
Profil folosit: staging
Numarul de operatii din fisier depaseste limita admisa. Limita:: 2, operatii: 6

% ./setenv -h
Utilizare: ./setenv options

Optiuni:
-h --help                  Afiseaza informatii referitoare la utilizarea comenzii
-m --max-environ-values    Numarul de variabile de mediu maxim
-a -–allowed-operations    Numar de operatii permise%     

% exit

Script done, output file is script.txt
*/