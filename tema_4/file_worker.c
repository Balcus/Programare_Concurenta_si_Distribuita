#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h> 
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

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
void* dirThread(void*);

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

void* inThread (void* args) {
    const inFile_t* file = (inFile_t*)args;
    char* buff;

    size_t bytes = myRead(file->filePath, &buff, file->size);
    printf("%s", buff);

    return (void*)bytes;
}

void* outThread(void* args) {
    const outFile_t* file = (outFile_t*)args;
    char* buff = "Write this to out_file";

    size_t bytes = myWrite(file->filePath, buff, strlen(buff));

    return (void*) bytes;
}

void* dirThread(void* args) {
    struct dirent* entry = (struct dirent*)args;
    struct stat fileStat;

    stat(entry->d_name, &fileStat);
    
    printf("%s %d", entry->d_name, (int)fileStat.st_size);

    return NULL;
}


int main() {
    pthread_t read_thread_id, write_thread_id, dir_thread_id;
    void *read_bytes,* wrote_bytes;

    inFile_t input_thread_args;
    input_thread_args.filePath = "/Users/balcus/Documents/Projects/PCD/teme/tema_4/in_file";
    input_thread_args.buffer = NULL;
    input_thread_args.size = 1024;

    outFile_t output_thread_args;
    output_thread_args.filePath = "/Users/balcus/Documents/Projects/PCD/teme/tema_4/out_file";
    output_thread_args.buffer = NULL;
    output_thread_args.size = 1024;

    pthread_create(&read_thread_id, NULL, inThread, &input_thread_args);
    pthread_create(&write_thread_id, NULL, outThread, &output_thread_args);
    // pthread_create(&dir_thread_id, NULL, dirThread, "./build");

    pthread_join(read_thread_id, &read_bytes);
    pthread_join(write_thread_id, &wrote_bytes);
    // pthread_join(dir_thread_id, NULL);

    if ((size_t)read_bytes == 0) {
        fprintf(stderr, "Eroare la citirea din fisier\n");
    }

    if ((size_t)wrote_bytes == 0) {
        fprintf(stderr, "Eroare la scrierea in fisier\n");
    }
}