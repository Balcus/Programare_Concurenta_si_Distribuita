// folosit pentru prevenirea warning-urilor clang-tidy pentru constante nedefinite
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

// structuri reprezentand argumentele de intrare pentru thread-urile de intrare si respectiv iesire
// aceleasi ca is in enuntul din laborator
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

// functie care sa citesasca un numar de bytes (size) dintr-un fisier intr-un buffer
// functia va returna numarul de bytes cititi din fisier
size_t myRead(char* path, char** buffer, size_t size) {
    // incercam sa deschidem fisierul in modul read only
    int fd = open(path, O_RDONLY);

    // in cazul unei erori la deschidere, afisam eroarea si
    // returnam 0, reprezentand numarul de bytes cititi din fisier
    if (fd == -1) {
        perror("Eroare la apelul de sistem open() ");
        return 0;
    }

    // alocam buffer-ului size bytes
    *buffer = malloc(size);

    // in cazul unei erori la alocarea de memorie, afisam eroarea, inchidem fisierul si returnam 0 bytes cititi
    if (*buffer == NULL) {
        perror("Eroare la apelul de sistem malloc()");
        close(fd);
        return 0;
    }

    // in variabila bytes salvam numarul de bytes cititi din fisier, valoare returnata de apelul read
    ssize_t bytes = read(fd, *buffer, size);

    // in cazul in care read a returnat valoare -1, a existat o eroare asa ca afisam eroarea,
    // eliberam memoria alocata pentru buffer si inchidem fisierul dupa care returnam 0 bytes cititi
    if (bytes == -1) {
        perror("Eroare la apelul de sistem read() ");
        close(fd);
        free(*buffer);
        return 0;
    }

    // in cazul in care nu am avut erori, inchidem fisierul si returnam numarul de bytes cititi.
    close(fd);
    return bytes;
}

// functie care scrie intr-un fisier dat ca parametru prin cale size bytes din continutul buffer-ului si returneaza numarul de bytes scrisi.
size_t myWrite(char* path, char* buffer, size_t size) {
    // deschidem fisierul in modul de scriere, in cazul in care exista deja continut in fisier, acesta va fi sters
    int fd = open(path, O_WRONLY | O_TRUNC);;

    // in cazul unei erori la deschidere, afisam eroarea si returnam 0
    // reprezentand numarul de bytes scrisi in fisier
    if (fd == -1) {
        perror("Eroare la apelul de sistem open() ");
        return 0;
    }

    // in variabila bytes salvam numarul de bytes scrisi in fisier, valoare returnata de apelul write
    ssize_t bytes = write(fd, buffer, size);

    // in cazul in care write a returnat valoare -1, a existat o eroare asa ca afisam eroarea,
    // inchidem fisierul dupa care returnam 0 bytes cititi
    if (bytes == -1) {
        perror("Eroare la apelul de sistem write() ");
        close(fd);
        return 0;
    }

    // in cazul in care nu am avut erori, inchidem fisierul si returnam numarul de bytes scrisi.
    close(fd);
    return bytes;
}

// thread care se va ocupa de citirea din in fisier, primeste ca parametru un void* args care va fi mai apoi
// cast in structura definita mai sus inFile_t* si returneaza numarul de bytes cititi din fisier ca void*
void* inThread (void* args) {
    // facem cast la structura dorita
    const inFile_t* file = (inFile_t*)args;
    // initializam buffer-ul trimis functiei de citire cu null
    char* buff = NULL;

    // apelam functia de citire definita de noi
    size_t bytes = myRead(file->filePath, &buff, file->size);

    // in cazul in care nu s-au citit bytes, returnam valoarea 0 (0 bytes cititi de thread)
    if ((int)bytes == 0) {
        return (void*)0;
    } else {
        // in cazul in care s-au citit bytes, afisam continutul citit din buffer
        printf("input_file content: %s\n", buff);
    }

    // eliberam resursele folosite de buffer
    free(buff);

    // returnam numarul de bytes citit
    return (void*)bytes;
}

// thread care se va ocupa de scrierea in fisier, primeste ca parametru un void* args care va fi mai apoi
// cast in structura definita mai sus outFile_t* si returneaza numarul de bytes scrisi in fisier ca void*
void* outThread(void* args) {
    // facem cast la structura dorita
    const outFile_t* file = (outFile_t*)args;

    // scriem in buffer mesajul pe care vrem sa il scriem
    char* buff = "Write this to out_file";

    // apelam funcita de scriere definita de noi folosind pentru size marimea buffer-ului
    size_t bytes = myWrite(file->filePath, buff, strlen(buff));

    // returnam numarul de bytes scrisi
    return (void*) bytes;
}

// cod executat de thread-urile create pentru fisiere
// primim ca argument calea catre fisier si afisam cateva stats despre fisier
// functia nu va returna nimic (folosim NULL pentru a nu primi eroarea: Non-void function 'fileThread' should return a valueclang(-Wreturn-mismatch))
void* fileThread(void* args) {
    char* fullPath = (char*)args;
    struct stat fileStat;

    // in cazul in care apare o eroare la optinerea stats-urilor fisierului, afisam eroare, eliberam bifferul folosit pentru cale si returnam NULL
    if (stat(fullPath, &fileStat) == -1) {
        perror("stat");
        free(fullPath);
        return NULL;
    }

    // afisam stats despre fisier: mod, path, size, inode number
    printf("[%lo] name: %s size: %lld bytes inode number: %ld\n", (unsigned long) fileStat.st_mode, fullPath, (long long) fileStat.st_size, (long) fileStat.st_ino);

    // eliberam bufferul pentru cale si returnam NULL
    free(fullPath);
    return NULL;
}

// functie folosita pentru parcurgerea in mod recursiv a unui director
// functia primeste ca parametru path-ul spre un director,
// il deschide si in cazul in care printre entry-uri se gaseste un alt director,
// se autoapeleaza folosind calea spre subdirector
void read_dir_recurs(const char* dirPath) {
    DIR* dir;
    struct dirent* entry;

    // incercam sa deschidem directorul, in caz de eroare afisam mesajul si returnam
    if ((dir = opendir(dirPath)) == NULL) {
        perror("Eroare la deschiderea directorului");
        return;
    }

    // cat timp putem citi intrarile directorului
    while ((entry = readdir(dir)) != NULL) {
        // buffer in care formam calea spre entry-ul din director
        // va fi folosit pentru fi trimis ca parametru la recursiune
        char fullPath[BUFFER_SIZE];

        // ignoram . si ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // verificam data putem formata si pune in bufferul fullPath calea spre entry-ul directorului, in cazul in care nu putem afisam eroarea si continuam la urmatorul entry
        if (snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name) < 0) {
            perror("Eroare la apelul snprintf() ");
            continue;
        }

        // daca dam de un director, apelam pe calea spre acel director functia read_dir_recurs
        if (entry->d_type == DT_DIR) {
            read_dir_recurs(fullPath);
        } else {
            // in cazul in care dam de un fisier, executam pe un thread nou functia de stat fisier
            pthread_t file_thread_id;
            // creeam thread-ul care sa execute functia de stat fisier
            pthread_create(&file_thread_id, NULL, fileThread, (void*)strdup(fullPath));
            // punem thread-ul in detach mode deoarece, functia fiind recursiva, e greu sa ne dam seama unde ar trebui sa se faca join-ul
            pthread_detach(file_thread_id);
        }
    }

    // inchidem directorul
    closedir(dir);
}

// functie folosita pentru crearea ierarhiei de directoare
// s-au folosit constantele S_IROTH, S_IXOTH, S_IXGRP, S_IWOTH, S_IRWXU pentru evitarea warning-urilor clang-tidy
void build_d1() {
    // creem directorul d1 cu perms: d------r-x
    mkdir("./d1", S_IROTH | S_IXOTH);

    // creem directorul d2 ca subdirector al d1 cu perms: d------rw-
    mkdir("./d1/d2", S_IROTH | S_IWOTH);

    // creem directorul d3 ca subdirector al d2 cu perms: dr-x-wx-w-
    mkdir("./d1/d2/d3", S_IRUSR | S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH);

    int fd;
    // folosim open cu O_CREAT pentru a crea fisierul f4 cu perms: -rwx--xr--
    fd = open("./d1/d2/d3/f4", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IXGRP | S_IROTH);
    if (fd != -1) {
        close(fd);
    }

    // folosim open cu O_CREAT pentru a crea fisierul f1 cu perms: -------rwx
    fd = open("./d1/d2/f1", O_CREAT | O_WRONLY | O_TRUNC, S_IROTH | S_IWOTH | S_IXOTH);
    if (fd != -1) {
        close(fd);
    }

    // folosim open cu O_CREAT pentru a crea fisierul f2 cu perms: -------r--
    fd = open("./d1/f2", O_CREAT | O_WRONLY | O_TRUNC, S_IROTH);
    if (fd != -1) {
        close(fd);
    }

    // folosim open cu O_CREAT pentru a crea fisierul f3 cu perms: --------wx
    fd = open("./d1/f3", O_CREAT | O_WRONLY | O_TRUNC, S_IWOTH | S_IXOTH);
    if (fd != -1) {
        close(fd);
    }

    // creem un link simbolic intre f1 si f4
    symlink("d2/f1", "./d1/f4");

    // creem un hard link intre f1 si f5
    link("./d1/d2/f1", "./d1/f5");
}


int main() {
    pthread_t read_thread_id, write_thread_id, dir_thread_id;
    void *read_bytes,* wrote_bytes;

    // creem structura de directoare
    build_d1();

    // initializam argumentele trimise ca input pentru thread-ul de citire
    inFile_t input_thread_args;
    input_thread_args.filePath = "./in_file";
    input_thread_args.buffer = NULL;
    input_thread_args.size = BUFFER_SIZE;

    // initializam argumentele trimise ca input pentru thread-ul de scriere
    outFile_t output_thread_args;
    output_thread_args.filePath = "./out_file";
    output_thread_args.buffer = NULL;
    output_thread_args.size = BUFFER_SIZE;

    // creem thread-ul de citire
    pthread_create(&read_thread_id, NULL, inThread, &input_thread_args);

    // creem thread-ul de scriere
    pthread_create(&write_thread_id, NULL, outThread, &output_thread_args);

    // invocam funcita de citire recursiva de directoare pe ierarhia de directoare creata mai sus
    read_dir_recurs("./d1");

    // asteptam finalizarea thread-urilor de citire si scriere
    pthread_join(read_thread_id, &read_bytes);
    pthread_join(write_thread_id, &wrote_bytes);

    // in cazul in care nu s-a citit nimic din fiser, afisam un mesaj de warning
    if ((size_t)read_bytes == 0) {
        if (fprintf(stderr, "0 bytes cititi din fisier\n") < 0) {
            perror("Eroare la apelul fprintf() ");
        }
    }

    // in cazul in care nu s-a citit nimic din fiser, afisam un mesaj de warning
    if ((size_t)wrote_bytes == 0) {
        if(fprintf(stderr, "0 bytes scrisi in fisier\n") < 0) {
            perror("Eroare la apelul fprintf() ");
        }
    }

    return 0;
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