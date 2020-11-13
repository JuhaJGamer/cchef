#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

IdHashMap createidhmap(size_t size);
void delidhmap(IdHashMap* map);
size_t hmhash(const char* key);
int keyexistsidhmap(const char* key, const IdHashMap* map);
int keyexistsidhmapeq(const char* key, const IdHashMap* map);
TokenList lex_line(const char* line, size_t size, TokenMaps* tokenmaps);
Token lex_word(const char* word, size_t length, TokenMaps* tokenmaps);
void pushidhmap(const char* key, unsigned int id, IdHashMap* map);
void resizeidhmap(size_t newsize, IdHashMap* map);
unsigned int valueofidhmap(const char* key, const IdHashMap* map);
size_t sizehidmap(const IdHashMap* map);

static const int cmdlen = 1;
static const char* commands[1] = { "Ingredients "};


IdHashMap createidhmap(size_t size) {
    return (IdHashMap){ malloc(sizeof(char*)*size), malloc(sizeof(size_t)*size), size };
}

TokenMaps createlextokenmaps() {
    TokenMaps tokenmaps = (TokenMaps) {.identifiers = createidhmap(1), .commands = createidhmap(cmdlen), .measures = createidhmap(1)};
    for(size_t i = 0; i < cmdlen; i++) {
        pushidhmap(commands[i], i, &tokenmaps.commands);
    }
    return tokenmaps;
}

TokenListList createtokenll() {
    return (TokenListList) { NULL, NULL };
}

TokenList createtokenl() {
    return (TokenList) { NULL, nulltoken };
}

void delidhmap(IdHashMap* map) {
    free(map->ids);
    for(char** key = map->keys; key - map->keys < map->size; key++) {
        free(*key);
    }
    free(map->keys);
    map->size=0;
}

TokenListList* headtokenll(TokenListList tokenll) {
    TokenListList* head = &tokenll;
    while(head->next != NULL)
        head = head->next;
    return head;
}

TokenList* headtokenl(TokenList tokenll) {
    TokenList* head = &tokenll;
    while(head->next != NULL)
        head = head->next;
    return head;
}

size_t hmhash(const char* key) {
    size_t hash = 5381;
    int c;
    while((c = *key++))
        hash = (((hash << 5) + hash) + c) % UINT_MAX;

    return hash;
}


int keyexistsidhmap(const char* key, const IdHashMap* map) {
    return map->keys[hmhash(key) % map->size] != NULL; 
}

int keyexistsidhmapeq(const char* key, const IdHashMap* map) {
    return map->keys[hmhash(key) % map->size] != NULL && !strcmp(map->keys[hmhash(key) % map->size], key); 
}

TokenList lex_line(const char* line, size_t size, TokenMaps* tokenmaps) {
    fprintf(stderr, "lex_line: %lu\n", size);
    TokenList tokenl = createtokenl();
    const char* wordbaseptr = line;
    for(const char* wordptr = wordbaseptr; wordptr-wordbaseptr < size; wordptr++) {
        if(*wordptr == ' ' || *wordptr == '.') {
            pushtokenl(lex_word(wordbaseptr, wordptr-wordbaseptr, tokenmaps), &tokenl);
            wordbaseptr = wordptr+1;
        }
    } 
    return tokenl;
}

Token lex_word(const char* word, size_t length, TokenMaps* tokenmaps) {
    fprintf(stderr, "lex_word: %lu\n", length);
    char* nw = malloc(sizeof(char)*(length));
    strncpy(nw,word,length);
    nw[length] = '\0';
    printf("%s\n", nw);

    if(keyexistsidhmapeq(nw, &tokenmaps->commands)) {
        fprintf(stderr, "FOUND COMMAND\n");
        return (Token) { valueofidhmap(nw, &tokenmaps->commands), TokenCmd };
    }
    return nulltoken;
}

TokenListList lex(const char* in, TokenMaps* tokenmaps) {
    TokenListList tokenll = createtokenll();
    const char* linebaseptr = in;
    for(const char* lineptr = linebaseptr; *lineptr != '\0'; lineptr++) {
        if(*lineptr == '\n') {
            if(lineptr != linebaseptr)
                pushtokenll(lex_line(linebaseptr, lineptr-linebaseptr, tokenmaps), &tokenll);
            linebaseptr = lineptr+1;
        }
    }
    return tokenll;
}

void pushidhmap(const char* key, unsigned int id, IdHashMap* map) {
    if(sizehidmap(map) > map->size) {
        resizeidhmap(map->size+1, map);
    }
    if(keyexistsidhmap(key,map)) {
        errno = EBADSLT;
        return;
    }
    else {
        map->ids[hmhash(key) % map->size] = id;
        map->keys[hmhash(key) % map->size] = malloc(strlen(key));
        *map->keys[hmhash(key) % map->size] = *key;
    }
}

void pushtokenll(TokenList item, TokenListList* tokenll) {
    TokenListList* head = headtokenll(*tokenll);
    head->next = malloc(sizeof(TokenListList));
    *head->next = (TokenListList) { NULL, item };
}

void pushtokenl(Token item, TokenList* tokenl) {
    TokenList* head = headtokenl(*tokenl);
    head->next = malloc(sizeof(TokenList));
    *head->next = (TokenList) { NULL, item };
}

void resizeidhmap(size_t newsize, IdHashMap* map) {
    IdHashMap newmap = createidhmap(newsize);
    for(int i = 0; i < map->size; i++) {
        pushidhmap(map->keys[i], map->ids[i], &newmap);
    }
    delidhmap(map);
    *map = newmap;
}

unsigned int valueofidhmap(const char* key, const IdHashMap* map) {
    unsigned int value = map->ids[hmhash(key) % map->size]; 
    return value;
}

size_t sizehidmap(const IdHashMap* map) {
    size_t n = map->size;
    for(size_t i = 0; i < map->size; i++) {
        if(map->keys[i] != NULL) 
            n--;
    }
    return n; 
}

