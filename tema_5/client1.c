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

#define BUFFER_SIZE 256
#define PORT 8001

void func(int sockfd) {
    char buff[BUFFER_SIZE];
    int n;
    while (1) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");

        n = 0;

        while ((buff[n++] = getchar()) != '\n')
            ;

        write(sockfd, buff, sizeof(buff));

        bzero(buff, sizeof(buff));

        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);

        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
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

    func(sockfd);

    close(sockfd);
}