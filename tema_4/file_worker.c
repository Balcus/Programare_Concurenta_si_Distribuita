#define _DEFAULT_SOURCE

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h> 
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

#define MAX_DIR_THREADS 10
#define BUFFER_SIZE 1024

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
void build_d1();

size_t myRead(char* path, char** buffer, size_t size) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Eroare la apelul de sistem open() ");
        return 0;
    }

    *buffer = malloc(size);
    if (*buffer == NULL) {
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
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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
    char* buff = NULL;

    size_t bytes = myRead(file->filePath, &buff, file->size);
    if (buff == NULL) {
        return (void*)0;
    } else {
        printf("input_file content: %s\n", buff);
    }

    return (void*)bytes;
}

void* outThread(void* args) {
    const outFile_t* file = (outFile_t*)args;
    char* buff = "Write this to out_file";

    size_t bytes = myWrite(file->filePath, buff, strlen(buff));

    return (void*) bytes;
}

void* fileThread(void* args) {
    char* fullPath = (char*)args;
    struct stat fileStat;

    if (stat(fullPath, &fileStat) == -1) {
        perror("stat");
        free(fullPath);
        return NULL;
    }

    printf("[%lo] name: %s size: %lld bytes inode number: %ld\n", (unsigned long) fileStat.st_mode, fullPath, (long long) fileStat.st_size, (long) fileStat.st_ino);
    free(fullPath);

    return NULL;
}


void read_dir_recurs(const char* dirPath) {
    DIR* dir;
    struct dirent* entry;

    if ((dir = opendir(dirPath)) == NULL) {
        perror("Eroare la deschiderea directorului");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char fullPath[BUFFER_SIZE];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name) < 0) {
            perror("Eroare la apelul snprintf() ");
            continue;
        }

        if (entry->d_type == DT_DIR) {
            read_dir_recurs(fullPath);
        } else {
            pthread_t file_thread_id;
            pthread_create(&file_thread_id, NULL, fileThread, (void*)strdup(fullPath));
            pthread_detach(file_thread_id);
        }
    }

    closedir(dir);
}

void build_d1() {
    mkdir("./d1", S_IROTH | S_IXOTH);
    mkdir("./d1/d2", S_IROTH | S_IWOTH);
    mkdir("./d1/d2/d3", S_IRUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH);

    int fd;
    fd = open("./d1/d2/d3/f4", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IXGRP | S_IROTH);
    if (fd != -1) {
        close(fd);
    }

    fd = open("./d1/d2/f1", O_CREAT | O_WRONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IXOTH);
    if (fd != -1) {
        close(fd);
    }

    fd = open("./d1/f2", O_CREAT | O_WRONLY | O_TRUNC, S_IROTH);
    if (fd != -1) {
        close(fd);
    }

    fd = open("./d1/f3", O_CREAT | O_WRONLY | O_TRUNC, S_IWOTH | S_IXOTH);
    if (fd != -1) {
        close(fd);
    }

    symlink("d2/f1", "./d1/f4");
    link("./d1/d2/f1", "./d1/f5");
}


int main() {
    pthread_t read_thread_id, write_thread_id, dir_thread_id;
    pthread_t dir_threads[MAX_DIR_THREADS];

    void *read_bytes,* wrote_bytes;

    build_d1();

    inFile_t input_thread_args;
    input_thread_args.filePath = "./in_file";
    input_thread_args.buffer = NULL;
    input_thread_args.size = BUFFER_SIZE;

    outFile_t output_thread_args;
    output_thread_args.filePath = "./out_file";
    output_thread_args.buffer = NULL;
    output_thread_args.size = BUFFER_SIZE;

    pthread_create(&read_thread_id, NULL, inThread, &input_thread_args);
    pthread_create(&write_thread_id, NULL, outThread, &output_thread_args);
    read_dir_recurs("./d1");

    pthread_join(read_thread_id, &read_bytes);
    pthread_join(write_thread_id, &wrote_bytes);

    if ((size_t)read_bytes == 0) {
        if (fprintf(stderr, "Eroare la citirea din fisier\n") < 0) {
            perror("Eroare la apelul fprintf() ");
            exit(1);
        }
    }

    if ((size_t)wrote_bytes == 0) {
        if(fprintf(stderr, "Eroare la scrierea in fisier\n") < 0) {
            perror("Eroare la apelul fprintf() ");
            exit(1);
        }
    }
}

/*

root@f944939cbc1b:/app# clang-tidy main.c -- -std=c17
8 warnings generated.
Suppressed 8 warnings (8 in non-user code).
Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.


root@f944939cbc1b:/app# ./main
input_file content: input

[100007] name: ./d1/f4 size: 0 bytes inode number: 49002
[100004] name: ./d1/f2 size: 0 bytes inode number: 49003
[100007] name: ./d1/f5 size: 0 bytes inode number: 49002
[100003] name: ./d1/f3 size: 0 bytes inode number: 49018
[100714] name: ./d1/d2/d3/f4 size: 0 bytes inode number: 49001

*/