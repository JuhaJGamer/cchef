#include <stdlib.h>

enum { TokenNull, TokenCmd, TokenBowl, TokenDish, TokenIng, TokenLiteral, TokenMeasure, TokenMeasureType, TokenSeparator };
enum { TSeparatorStop };
enum { TMeasureGram, TMeasureKg, TMeasurePinch, TMeasureMl, TMeasureL, TMeasureDash, TMeasureCup, TMeasureTsp, TMeasureTblsp };
enum { TMeasureTypeHeaped, TMeasureTypeLevel };

typedef struct{
    char** keys;
    unsigned int* ids;
    size_t size;
} IdHashMap;

typedef struct {
    unsigned int id;
    int type;
} Token;

static const Token nulltoken = { 0, TokenNull };

typedef struct {
    IdHashMap identifiers;
    IdHashMap commands;
    IdHashMap measures;
} TokenMaps;

typedef struct TokenList TokenList;
struct TokenList {
    TokenList* next;
    Token token;
};

typedef struct TokenListList TokenListList;
struct TokenListList {
    TokenListList* next;
    TokenList list;
};


TokenListList lex(const char* prog, TokenMaps* tokenmaps);
TokenMaps createlextokenmaps();
TokenListList createtokenll();
TokenList createtokenl();
void pushtokenll(TokenList value,TokenListList* list);
void pushtokenl(Token value,TokenList* list);
TokenListList* headtokenll(TokenListList list);
TokenList* headtokenl(TokenList list);
TokenListList* indextokenll(TokenList list);
TokenList* indextokenl(TokenList list);

