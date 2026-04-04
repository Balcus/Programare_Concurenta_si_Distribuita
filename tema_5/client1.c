/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 5: client1.c
 */

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <fcntl.h>

#define BUFFER_SIZE 256
#define PORT 8001
#define CREDENTIALS_FILE "/tmp/credentials.txt"

int auth(char*, char*);
void show_menu();
void handle_comm(int);

int auth(char *username, char *password) {
    int fd = open(CREDENTIALS_FILE, O_RDONLY);
    if (fd == -1) {
        return 0;
    }

    char buff[BUFFER_SIZE];
    int n = read(fd, buff, sizeof(buff) - 1);
    close(fd);

    if (n <= 0) {
        return 0;
    }

    buff[n] = '\0';

    char *line = strtok(buff, "\n");
    while (line != NULL) {
        char valid_username[BUFFER_SIZE], valid_password[BUFFER_SIZE];
        if (sscanf(line, "%s %s", valid_username, valid_password) == 2) {
            if (strcmp(username, valid_username) == 0 && strcmp(password, valid_password) == 0) {
                return 1;
            }
        }
        line = strtok(NULL, "\n");
    }
    return 0;
}

void show_menu() {
    printf("\n>>> 1. Send Message\n");
    printf(">>> 2. Request Time\n");
    printf(">>> 3. Request User\n");
    printf(">>> 4. Execute Command\n");
    printf(">>> 5. Close Connection\n");
}

void handle_comm(int s) {
    char b[BUFFER_SIZE];
    char sel[10];

    while (1) {
        show_menu();
        printf("Select: ");
        
        fgets(sel, sizeof(sel), stdin);
        int option = atoi(sel);

        switch (option) {
            case 1:
                printf(">>> Send Message\n  Enter message: ");
                fgets(b, BUFFER_SIZE, stdin);
                write(s, b, strlen(b));
                break;
            case 2:
                printf(">>> Request Time\n");
                write(s, "time\n", 5);
                break;
            case 3:
                printf(">>> Request User\n");
                write(s, "user\n", 5);
                break;
            case 4:
                printf(">>> Execute Command\n  Enter cmd: ");
                fgets(b, BUFFER_SIZE, stdin);
                write(s, b, strlen(b));
                break;
            case 5:
                write(s, "exit\n", 5);
                printf("Client Exit...\n");
                return;
            default:
                printf("Optiune invalida.\n");
                continue;
        }

        memset(b, 0, BUFFER_SIZE);
        read(s, b, BUFFER_SIZE);
        printf("::: %s", b);
    }
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Eroare la crearea socket-ului ");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("Eroare la conectarea cu server-ul ");
        close(sockfd);
        exit(1);
    }

    char username[BUFFER_SIZE], password[BUFFER_SIZE];
    printf("Username: "); scanf("%s", username);
    printf("Password: "); scanf("%s", password);
    getchar();

    if (auth(username, password) == 1) {
        handle_comm(sockfd);
    }

    close(sockfd);
}