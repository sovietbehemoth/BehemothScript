#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <string.h>
#include <ctype.h>

#pragma once

#if defined(WIN32) || defined(_WIN32) 
    #include <io.h>
    #define F_OK 0
    #define access _access
#else
    #include <unistd.h>
#endif

#define CHAR sizeof(char)

#define UNDEFINED -1        //Used to terminate linked list.
#define OBJECT 0            //Stores node(s) of information within an enclosed block.
#define CCHAR 1             //Regular character.
#define WSPACE 2            //Whitespace.
#define LBREAK 3            //Line break.

#define NO_COMMENT -1
#define ONE_LINE_COMMENT 0
#define MULTI_LINE_COMMENT 1

typedef struct Tok tok_t;
typedef struct Match match_t;

//Collection data structure for token pairing.
typedef struct Tok {
    match_t *parenthesis;
    match_t *square_braces;
    match_t *curly_braces;
} tok_t;

tok_t *tokenizer(char *input);
int find_pair(int index, match_t *matches);
bool escaped(char *input, int n);

char **lexer_init(char *buffer);

void syntax_error(char *msg, int line, int c);

__attribute__((malloc)) void * _calloc(size_t size, size_t count);

void * _memcpy(void *dest, void *src, size_t size);

extern int heap;