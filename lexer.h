#include <stdlib.h>

enum { TokenEmpty, TokenInteger, TokenSeparator, TokenWord };

typedef struct Token Token;
typedef struct TokenizedLine TokenizedLine;
typedef struct TokenizedProg TokenizedProg;

struct Token {
    int type;
    int id;
    void* data;
};

struct TokenizedLine {
    Token* tokenv;
    size_t tokenc;
};

struct TokenizedProg {
    TokenizedLine* linev;
    size_t linec;
};

TokenizedProg lex(const char* prog);
