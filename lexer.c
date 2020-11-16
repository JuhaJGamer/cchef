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
    enum { StateNone, StateKeyword, StateIngredient, StateMeasure }; 
    int state = StateNone;

    LinkedTokenizedLine baseltokenizedline = create_ltokenizedline();
    LinkedTokenizedLine* ltokenizedline = &baseltokenizedline;
    LinkedTokenizedProg baseltokenizedprog = create_ltokenizedprog();
    LinkedTokenizedProg* ltokenizedprog = &baseltokenizedprog;
    Token token = create_token();

    for(TokenizedLine* linep = prog.linev; linep-prog.linev < prog.linec; linep++) {
        for(Token* tokenp = linep->tokenv; tokenp-linep->tokenv < linep->tokenc; tokenp++) {
            if(state == StateNone) {
                switch(tokenp->type) {
                    case TokenInteger:
                        state = StateMeasure;
                        append_ltokenizedline(*tokenp, &ltokenizedline);
                        continue;
                    case TokenWord:
                        state = StateKeyword;
                        break;
                }
            }        
            if(tokenp->type == TokenSeparator && tokenp->id == 0) {
                token = *tokenp; 
                append_ltokenizedline(token, &ltokenizedline);
                token = create_token();
                if(tokenp-linep->tokenv < linep->tokenc-1) {
                    TokenizedLine tokenizedline = collapse_ltokenizedline(baseltokenizedline);
                    free_ltokenizedline(baseltokenizedline.next);
                    append_ltokenizedprog(tokenizedline, &ltokenizedprog);
                    baseltokenizedline = create_ltokenizedline();
                    ltokenizedline = &baseltokenizedline;
                    state = StateNone;
                }
                continue;
            }
            switch(state) {
                case StateMeasure:
                    if(tokenp->type != TokenWord) {
                        state = StateNone;
                        break;
                    } 
                    else if (!strcmp("g",tokenp->data)) {
                        token = (Token) { TokenMeasure, TokenMeasureG, NULL };
                    }
                    else if (!strcmp("kg",tokenp->data)) {
                        token = (Token) { TokenMeasure, TokenMeasureKg, NULL };
                    }
                    else if (!strncmp("pinch",tokenp->data,5)) {
                        token = (Token) { TokenMeasure, TokenMeasurePinch, NULL };
                    }
                    else if (!strcmp("ml",tokenp->data)) {
                        token = (Token) { TokenMeasure, TokenMeasureMl, NULL };
                    }
                    else if (!strcmp("l",tokenp->data)) {
                        token = (Token) { TokenMeasure, TokenMeasureL, NULL };
                    }
                    else if (!strncmp("dash",tokenp->data, 4)) {
                        token = (Token) { TokenMeasure, TokenMeasureDash, NULL };
                    }
                    else if (!strncmp("cup",tokenp->data, 3)) {
                        token = (Token) { TokenMeasure, TokenMeasureCup, NULL };
                    }
                    else if (!strncmp("teaspoon",tokenp->data,8)) {
                        token = (Token) { TokenMeasure, TokenMeasureTsp, NULL };
                    }
                    else if (!strncmp("tablespoon",tokenp->data,10)) {
                        token = (Token) { TokenMeasure, TokenMeasureTblsp, NULL };
                    } 
                    else if (!strcmp("heaped",tokenp->data)) {
                        token = (Token) { TokenMeasure, TokenMeasureHeaped, NULL };
                    }
                    else if (!strcmp("level",tokenp->data)) {
                        token = (Token) { TokenMeasure, TokenMeasureLevel, NULL };
                    }
                    else {
                        state = StateIngredient;
                        tokenp--;
                        break;
                    }
                    append_ltokenizedline(token, &ltokenizedline);
                    token = create_token();
                    break;
                case StateIngredient:
                    if(!strcmp("from", tokenp->data)
                            || !strcmp("into", tokenp->data)
                            || !strcmp("to", tokenp->data)
                            || !strcmp("until", tokenp->data)) {
                        state = StateKeyword;
                        tokenp--;
                        append_ltokenizedline(token, &ltokenizedline);
                        token = create_token();
                        continue;
                    }
                    if ((char*)token.data == NULL) {
                        token.type = TokenIngredient;
                        token.data = malloc(sizeof(char)*strlen(tokenp->data)+1);
                        strcpy(token.data,tokenp->data);
                    } else {
                        size_t oldlen = strlen(token.data);
                        token.data = realloc(token.data, oldlen+strlen(tokenp->data)+1);
                        ((char*)token.data)[oldlen] = ' ';
                        strcpy(token.data+oldlen+1,tokenp->data);
                    }
                    break;
                case StateKeyword:
                    if(tokenp->type == TokenInteger) {
                        append_ltokenizedline(*tokenp, &ltokenizedline);
                        token = create_token();
                        break;
                    }
                    token.type = TokenKeyword;
                    if ((char*)token.data == NULL) {
                        token.data = malloc(sizeof(char)*strlen(tokenp->data)+1);
                        strcpy(token.data,tokenp->data);
                    } else {
                        size_t oldlen = strlen(token.data);
                        token.data = realloc(token.data, oldlen+strlen(tokenp->data)+1);
                        ((char*)token.data)[oldlen] = ' ';
                        strcpy(token.data+oldlen+1,tokenp->data);
                    }
                    if(strcmp("baking", tokenp->data)
                            && strcmp("mixing", tokenp->data)) {
                        append_ltokenizedline(token,&ltokenizedline);
                        token = create_token();
                    }
                    if(!strcmp("Take", tokenp->data)
                            || !strcmp("Put", tokenp->data)
                            || !strcmp("Fold", tokenp->data)
                            || (!strcmp("Add", tokenp->data) && tokenp-linep->tokenv < linep->tokenc+1 && strcmp("dry", tokenp++->data))
                            || !strcmp("Remove", tokenp->data)
                            || !strcmp("Combine", tokenp->data)
                            || !strcmp("Divide", tokenp->data)
                            || !strcmp("Stir", tokenp->data)) {
                        state = StateIngredient;
                        break;
                    }
                    break;
            }
            
        }
        append_ltokenizedline(token, &ltokenizedline);
        token = create_token();
        TokenizedLine tokenizedline = collapse_ltokenizedline(baseltokenizedline);
        free_ltokenizedline(baseltokenizedline.next);
        append_ltokenizedprog(tokenizedline, &ltokenizedprog);
        baseltokenizedline = create_ltokenizedline();
        ltokenizedline = &baseltokenizedline;
        state = StateNone;
    }

             
    TokenizedProg tokenized;
    // Flattening
    tokenized = collapse_ltokenizedprog(baseltokenizedprog);
    // Free everything
    free_ltokenizedline(baseltokenizedline.next);
    free_ltokenizedprog(baseltokenizedprog.next);
    
    return tokenized;
}
