#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

typedef struct LinkedString LinkedString;
typedef struct LinkedTokenizedLine LinkedTokenizedLine;
typedef struct LinkedTokenizedProg LinkedTokenizedProg;

struct LinkedString {
    LinkedString* next;
    char c;
};

struct LinkedTokenizedLine {
    LinkedTokenizedLine* next;
    Token token;
};

struct LinkedTokenizedProg {
    LinkedTokenizedProg* next;
    TokenizedLine line;
};

TokenizedLine collapse_ltokenizedline(LinkedTokenizedLine line);
TokenizedProg collapse_ltokenizedprog(LinkedTokenizedProg prog);
LinkedTokenizedLine create_ltokenizedline();
LinkedTokenizedProg create_ltokenizedprog();
TokenizedProg lexer_parsing_pass(const char* prog);
TokenizedProg lexer_token_pass(TokenizedProg);

LinkedTokenizedLine create_ltokenizedline(Token token) {
    return (LinkedTokenizedLine) { NULL, token }; 
}

LinkedTokenizedProg create_ltokenizedprog(TokenizedLine line) {
    return (LinkedTokenizedProg) { NULL, line };
}

TokenizedLine collapse_ltokenizedline(LinkedTokenizedLine line) {
    size_t n = 0;
    for(const LinkedTokenizedLine* node = &line; node != NULL; node = node->next)
        n++;
    TokenizedLine collapsed_line = { malloc(sizeof(Token)*n), n };
    Token* tokenp = collapsed_line.tokenv;
    for(const LinkedTokenizedLine* node = &line; node != NULL; node = node->next)
        *tokenp++ = node->token;
    return collapsed_line;
}

TokenizedProg collapse_ltokenizedprog(LinkedTokenizedProg prog) {
    size_t n = 0;
    for(const LinkedTokenizedProg* node = &prog; node != NULL; node = node->next)
        n++;
    TokenizedProg collapsed_prog = { malloc(sizeof(TokenizedLine)*n), n };
    TokenizedLine* linep = collapsed_prog.linev;
    for(const LinkedTokenizedProg* node = &prog; node != NULL; node = node->next)
        *linep++ = node->line;
    return collapsed_prog;
}

TokenizedProg lex(const char* prog) {
    TokenizedProg tokenized;
    tokenized = lexer_parsing_pass(prog);
    tokenized = lexer_token_pass(tokenized);
    return tokenized;
}

/* Tokenizes program, but only recognizes tokens which start with distinct characters
 * Subject to second pass to tokenize properly, 
 * this is basically a cleanup/initial parsing pass */
TokenizedProg lexer_parsing_pass(const char* prog) {
    TokenizedProg tokenized;  

    LinkedTokenizedLine baseltokenizedline;
    LinkedTokenizedLine* ltokenizedline = &baseltokenizedline;
    LinkedTokenizedProg baseltokenizedprog = create_ltokenizedprog((TokenizedLine){ NULL, 0 });
    LinkedTokenizedProg* ltokenizedprog = &baseltokenizedprog;
    Token token = (Token){ 0, 0, NULL};

    enum { StateNone, StateEndLexeme, StateEndLine, StateNumeric, StateWord };
    int state = StateNone;

    for(const char* c = prog; *c != '\0'; c++) {
        if(state == StateNone) {
            if(*c >= '0' && *c <= '9') {
                state = StateNumeric; 
                token = (Token) { TokenInteger, 0, malloc(sizeof(int)) };
                *(int*)token.data = 0;
            }
            else if (*c >= 'A' && *c <= 'z') {
                state = StateWord;
                token = (Token){ TokenWord, 0, malloc(sizeof(char)) };
                *(char*)token.data = '\0';
            }
            else if (*c == '\n') {
                state = StateEndLine;
            }
            else if(*c == '.') {
                token = (Token){ TokenSeparator, 0, NULL };
                state = StateEndLexeme;
            }
        } else if(state != StateEndLexeme && state != StateEndLine) {
            switch(*c) {
                case '.':
                    c--;
                case ' ':
                    state = StateEndLexeme;
                    break;
                case '\n':
                    state = StateEndLine;
                    break;
                default:
                    break;
            }
        }
        switch(state) {
            case StateEndLine:
                if(ltokenizedline == &baseltokenizedline)
                    break;
                fprintf(stderr, "Line!\n");
                ltokenizedline->token = token;
                TokenizedLine tokenizedline = collapse_ltokenizedline(baseltokenizedline);
                for(LinkedTokenizedLine* node = baseltokenizedline.next; node != NULL; free(ltokenizedline)) {
                    ltokenizedline = node;
                    node = node->next;
                }
                baseltokenizedline = create_ltokenizedline((Token) { 0, 0 });
                ltokenizedline = &baseltokenizedline;
                ltokenizedprog->line = tokenizedline;
                ltokenizedprog->next = malloc(sizeof(LinkedTokenizedProg));
                *ltokenizedprog->next = create_ltokenizedprog((TokenizedLine){ NULL, 0 });
                ltokenizedprog = ltokenizedprog->next;
                token = (Token) { 0, 0, NULL };
                state = StateNone;
                break;
            case StateEndLexeme:
                fprintf(stderr, "Lexeme!\n");
                ltokenizedline->token = token; 
                ltokenizedline->next = malloc(sizeof(LinkedTokenizedLine));
                *ltokenizedline->next = create_ltokenizedline((Token){ 0, 0, NULL });
                ltokenizedline = ltokenizedline->next;
                token = (Token) { 0, 0, NULL };
                state = StateNone;
                break;
            case StateNumeric:
                *(int*)token.data *= 10;
                *(int*)token.data += *c-'0';
                break;
            case StateWord:
                {
                    char* oldstr = (char*)token.data;
                    size_t l = strlen(oldstr)+1;
                    token.data = malloc((l+1)*sizeof(char));
                    strcpy(token.data, oldstr);
                    ((char*)token.data)[l-1] = *c;
                    ((char*)token.data)[l] = '\0';
                    free(oldstr);
                    break;
                }
            default:
                break;
        }
    }
    tokenized = collapse_ltokenizedprog(baseltokenizedprog);
    for(LinkedTokenizedProg* node = baseltokenizedprog.next; node != NULL; free(ltokenizedprog)) {
        ltokenizedprog = node;
        node = node->next;
    }
    return tokenized;
}

/*
 * Tokenizes pre-tokenized "clean" program
 * Recognizes (some) context to turn "Word" into "Measure" or "Keyword"
 */
TokenizedProg lexer_token_pass(TokenizedProg prog) {
    // TODO: code
    return prog;
}
