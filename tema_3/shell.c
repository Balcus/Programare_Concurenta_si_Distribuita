#include "libconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define CONFIG_PATH "shell_config.cfg"

struct ParsedConfig {
    char* username;
    char* password;
    char* prompt;
};

struct Command {
    char* name;
    int argc;
    char* *argv;
};

struct ParsedConfig parse_config(config_t *cfg);
void destroy_parsed_config(struct ParsedConfig *pc);
int shell(struct ParsedConfig *pc);
void login(char * const username, char * const password, struct ParsedConfig *pc);
int system_2(char * const command);
int adatped_system(char * const commands);


struct ParsedConfig parse_config(config_t *cfg) {
    const char *prompt, *username, *password;

    if(!config_read_file(cfg, CONFIG_PATH)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(cfg), config_error_line(cfg), config_error_text(cfg));
        config_destroy(cfg);
        exit(1);
    }

    if(!config_lookup_string(cfg, "prompt", &prompt)) {
        fprintf(stderr, "Campul prompt lipseste din configurare\n");
        config_destroy(cfg);
        exit(1);
    }

    if(!config_lookup_string(cfg, "username", &username)) {
        fprintf(stderr, "Campul username lipseste din configurare\n");
        config_destroy(cfg);
        exit(1);
    }

    if(!config_lookup_string(cfg, "password", &password)) {
        fprintf(stderr, "Campul password lipseste din configurare\n");
        config_destroy(cfg);
        exit(1);
    }

    struct ParsedConfig parsed;
    parsed.username = strdup(username);
    parsed.password = strdup(password);
    parsed.prompt = strdup(prompt);

    return parsed;
}

void destroy_parsed_config(struct ParsedConfig *pc) {
    free(pc->username);
    free(pc->password);
    free(pc->prompt);
}

void login(char * const username, char * const password, struct ParsedConfig *pc) {
    if (strcmp(username, pc->username) != 0 || strcmp(password, pc->password) != 0) {
        fprintf(stderr, "Credentiale invalide\n");
        destroy_parsed_config(pc);
        exit(1);
    }
}

int system_2(char * const command) {

}

int adatped_system(char * const commands) {
    
}

int shell(struct ParsedConfig *pc) {
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];

    printf("username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    printf("password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    login(username, password, pc);

    char command[BUFFER_SIZE];
    while (1) {
        printf("%s ", pc->prompt);
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        command[strcspn(command, "\n")] = '\0';
    }
    return 0;
}

int main() {
    config_t cfg;
    config_init(&cfg);
    struct ParsedConfig pc = parse_config(&cfg);
    int status = shell(&pc);
    destroy_parsed_config(&pc);
    config_destroy(&cfg);
    return status;
}