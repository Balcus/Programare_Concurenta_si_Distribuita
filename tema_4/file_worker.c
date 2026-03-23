#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h> 
#include <fcntl.h>
#include <unistd.h>

typedef struct inFile {
    char* filePath;
    char* buffer;
    size_t size;
} inFile_t;

typedef struct outFile {
    char* filePath;
    char* buffer;
    size_t size;
} outFile_t;

size_t myRead(char*, char**, size_t);
size_t myWrite(char*, char*, size_t);
void* inThread (void*);
void* outThread(void*);
void* fileThread(void*);

size_t myRead(char* path, char** buffer, size_t size) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Eroare la apelul de sistem open() ");
        return 0;
    }

    *buffer = malloc(size);
    if (*buffer == 0) {
        perror("Eroare la apelul de sistem malloc()");
        close(fd);
        return 0;
    }

    ssize_t bytes = read(fd, *buffer, size);
    if (bytes == -1) {
        perror("Eroare la apelul de sistem read() ");
        close(fd);
        free(*buffer);
        return 0;
    }

    close(fd);
    return bytes;
}

size_t myWrite(char* path, char* buffer, size_t size) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Eroare la apelul de sistem open() ");
        return 0;
    }

    ssize_t bytes = write(fd, buffer, size);
    if (bytes == -1) {
        perror("Eroare la apelul de sistem write() ");
        close(fd);
        return 0;
    }

    close(fd);
    return bytes;
}

int main() {

}