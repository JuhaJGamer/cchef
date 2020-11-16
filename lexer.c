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
void append_ltokenizedline(Token token, LinkedTokenizedLine** line);
void append_ltokenizedprog(TokenizedLine line, LinkedTokenizedProg** prog);
TokenizedLine collapse_ltokenizedline(LinkedTokenizedLine line);
TokenizedProg collapse_ltokenizedprog(LinkedTokenizedProg prog);
LinkedTokenizedLine create_ltokenizedline();
LinkedTokenizedProg create_ltokenizedprog();
void free_ltokenizedline(LinkedTokenizedLine*);
void free_ltokenizedprog(LinkedTokenizedProg*);
TokenizedProg lexer_parsing_pass(const char* prog);
TokenizedProg lexer_token_pass(TokenizedProg);

void append_ltokenizedline(Token token, LinkedTokenizedLine** line) {
    (*line)->token = token;
    (*line)->next = malloc(sizeof(LinkedTokenizedLine));
    *(*line)->next = create_ltokenizedline();
    *line = (*line)->next;
}

void append_ltokenizedprog(TokenizedLine line, LinkedTokenizedProg** prog) {
    (*prog)->line = line;
    (*prog)->next = malloc(sizeof(LinkedTokenizedProg));
    *(*prog)->next = create_ltokenizedprog();
    *prog = (*prog)->next;
}

Token create_token() {
    return(Token) { 0, 0, NULL };
}

LinkedTokenizedLine create_ltokenizedline() {
    return (LinkedTokenizedLine) { NULL, create_token() }; 
}

LinkedTokenizedProg create_ltokenizedprog() {
    return (LinkedTokenizedProg) { NULL, (TokenizedLine){ NULL, 0 } };
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

void free_ltokenizedline(LinkedTokenizedLine* line) {
    LinkedTokenizedLine prevline;
    for(LinkedTokenizedLine* node = line; node != NULL; node = prevline.next) {
        prevline = *node;
        free(node);
    }
}

void free_ltokenizedprog(LinkedTokenizedProg* prog) {
    LinkedTokenizedProg prevelem;
    for(LinkedTokenizedProg* node = prog; node != NULL; node = prevelem.next) {
        prevelem = *node;
        free(node);
    }
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
    LinkedTokenizedProg baseltokenizedprog = create_ltokenizedprog();
    LinkedTokenizedProg* ltokenizedprog = &baseltokenizedprog;
    Token token = (Token){ 0, 0, NULL};

    enum { StateNone, StateEndLexeme, StateEndLine, StateNumeric, StateWord };
    int state = StateNone;

    for(const char* c = prog; *c != '\0'; c++) {
        // If not currently reading a lexeme, check if you could
        // And set the state and token accordingly
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
        } 
        // Lexeme-interrupting characters and their respective states
        // Only check these if lexeme isn't already interrupted
        else if(state != StateEndLexeme && state != StateEndLine) {
            switch(*c) {
                case '.':
                    c--; // Don't skip the character, we want to process it as its own token
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
                {
                    // Don't create empty lines
                    if(ltokenizedline == &baseltokenizedline)
                        break;
                    // Save, flatten line, and append it
                    ltokenizedline->token = token;
                    TokenizedLine tokenizedline = collapse_ltokenizedline(baseltokenizedline);
                    free_ltokenizedline(baseltokenizedline.next);
                    append_ltokenizedprog(tokenizedline, &ltokenizedprog);
                    // Set up next (empty) line
                    baseltokenizedline = create_ltokenizedline();
                    ltokenizedline = &baseltokenizedline;
                    // Reset token and state
                    token = (Token) { 0, 0, NULL };
                    state = StateNone;
                    break;
                }
            case StateEndLexeme:
                append_ltokenizedline(token, &ltokenizedline);
                // Reset token and state
                token = (Token) { 0, 0, NULL };
                state = StateNone;
                break;
            case StateNumeric:
                // append decimal digit
                *(int*)token.data *= 10;
                *(int*)token.data += *c-'0';
                break;
            case StateWord:
                {
                    // Extend string by 1 character and place *c there
                    // This is horrible 
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
    // Flattening
    tokenized = collapse_ltokenizedprog(baseltokenizedprog);
    // Free everything
    free_ltokenizedline(baseltokenizedline.next);
    free_ltokenizedprog(baseltokenizedprog.next);
    
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
