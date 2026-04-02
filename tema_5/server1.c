/*
 * Nume si prenume: Balcus Bogdan
 * IR3 2026, subgrupa 1
 * Tema 5: server1.c
 */

#include <time.h> // time(), ctime()
#include <pwd.h>  
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#define BUFFER_SIZE 256
#define PORT 8001

void handle_comm(int connfd) {
    char buff[BUFFER_SIZE];
    while (1) {
        memset(buff, 0, BUFFER_SIZE);
        int n = read(connfd, buff, BUFFER_SIZE);
        if (n <= 0) {
            printf("Clientul a inchis conexiunea.\n");
            break;
        }
        buff[strcspn(buff, "\n")] = 0;

        char response[BUFFER_SIZE];
        if (strcmp(buff, "time") == 0) {
            time_t t = time(NULL);
            snprintf(response, BUFFER_SIZE, "%s", ctime(&t));
        } else if (strcmp(buff, "user") == 0) {
            struct passwd *pw = getpwuid(getuid());
            snprintf(response, BUFFER_SIZE, "%s\n", pw->pw_name);
        } else {
            snprintf(response, BUFFER_SIZE, "echo :: %s\n", buff);
        }

        write(connfd, response, strlen(response));
    }
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Eroare la creearea socket-uluil ");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        perror("Eroare la bindig de socket pe adresa IP ");
        close(sockfd);
        exit(1);
    }

    if ((listen(sockfd, 5)) != 0) {
        perror("Eroare la apelul listen() ");
        close(sockfd);
        exit(1);
    }

    socklen_t len;
    len = sizeof(cli);

    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0) {
        perror("Eroare la apelul accept() ");
        close(sockfd);
        exit(1);
    }

    handle_comm(connfd);

    close(sockfd);
}