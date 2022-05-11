#include "include/BehemothScript.h"

#include <sys/stat.h>

int heap = 0;

void * _calloc(size_t count, size_t size)
{
    heap += size*count;
    return calloc(size, count);
}

void * _memcpy(void *dest, void *src, size_t size) 
{
    return memcpy(dest, src, size);
} 

char *read_file(char *pathname) {
    if (access( pathname, F_OK ) != 0) {
        fprintf(stderr, "\e[0;31mError\e[0;37m: No such file '%s'.\n", pathname);
        exit(1);
    } 

    struct stat phandler;

    stat(pathname, &phandler);

    int size = phandler.st_size;
    char *buffer = (char*) _calloc(size, CHAR);

    FILE * f = fopen(pathname, "r");

    read(fileno(f), buffer, size);

    return buffer;
}

//Warn if source has no BHS extensions per the standard.
void is_BHS(char *path) {
    char *extension = (char*) calloc(strlen(path), sizeof(char));

    bool getext = false;

    for (int i = 0; i < strlen(path); i++) {
        if (i > 0) {
            if (path[i] == '.' && path[i-1] != '/') {
                getext = true;
                continue;
            }
        }

        if (getext) {
            if (path[i] == '/') {
                break;
            } else {
                int len = strlen(extension);
                extension[len] = path[i];
            }
        }
    }

    if (!strcmp(extension, "bhs")) {
        free(extension);
    } else {
        free(extension);
        fprintf(stderr, "\e[0;35mWarning\e[0;37m: Source file does not have standard '.bhs' extension, it may not be a Behemoth script.\n");
    }
}

int main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, "\e[0;31mError\e[0;37m: Expected source file or option.\n");
        exit(1);
    }

    int source = -1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i], "-v")) {
                puts("BHS Interpreter v0.0.1");
                exit(EXIT_SUCCESS);
            }
        } else {
            source = i;
        }
    }

    if (source == -1) {
        fprintf(stderr, "\e[0;31mError\e[0;37m: Expected source file.\n");
        exit(1);
    }

    char *b = read_file(argv[source]);

    is_BHS(argv[source]);

    //tok_t * tok = tokenizer(b);
    lexer_init(b);

    //printf("Heap: %d\n", heap);
}