#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

typedef struct {
    char* output;
} Flags;

void compile_unit(FILE* f);
char* preprocess(const char* prog);

int main(int argc, char** argv) { 
    Flags flags;
    char* input[16];
    char** inputp = (char**)&input;
    for(char** arg = &argv[1]; arg-argv < argc; arg++) {
        if(!strcmp(*arg, "-o")) {
            flags.output = *++arg;
        }
        else {
            *inputp++ = *arg;
        }
    }
    if(flags.output == NULL) {
        flags.output = "a.out";
    }
    if(inputp == (char**)&input) {
        fprintf(stderr, "cchef: No input files\n");
        exit(1);
    }
    for(char** fpp = (char**)&input; fpp < inputp; fpp++) {
        FILE* f = fopen(*fpp, "r");
        if(errno) {
            err(errno, NULL);
            exit(1);
        }
        fprintf(stderr,"opened file %s\n", *fpp);
        compile_unit(f);
    }
    return 0;
}

void compile_unit(FILE* f) {
    fprintf(stderr,"compile start\n");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    char* prog = malloc(sizeof(char)*(fsize+1));
    fseek(f,0,SEEK_SET);
    fprintf(stderr, "read %lu chars\n",fread(prog, 1, fsize, f));
    if(errno) {
        err(errno, NULL);
        exit(1);
    }
    fprintf(stderr,"read file into buffer of size %lu\n", fsize);
    prog[fsize] = '\0';
    fprintf(stderr, "%s\n", prog);
    {
        char* tprog = preprocess(prog);
        free(prog);
        prog = tprog;
    }
    fprintf(stderr, "---\n");
    fprintf(stderr, "%s\n", prog);
    TokenizedProg tokenized = lex(prog);
    for(TokenizedLine* line = tokenized.linev; line-tokenized.linev < tokenized.linec; line++) {
        for(Token* token = line->tokenv; token-line->tokenv < line->tokenc; token++) {
            switch(token->type) {
                case TokenInteger:
                    fprintf(stderr, "[ Int %d ] ", *(int*)token->data);
                    break;
                case TokenSeparator:
                    fprintf(stderr, "[ Sep %c ] ", ". "[token->id]);
                    break;
                case TokenWord:
                    fprintf(stderr, "[ Word '%s' ] ", (char*)token->data);
                    break;
            }
        }
        fprintf(stderr, "\n");
    }
}

char* preprocess(const char* prog) {
    enum { StateTitle, StateComment, StateProg };
    int state = StateTitle;
    char* processed_prog = malloc(strlen(prog)*sizeof(char));
    char* process_p = processed_prog;
    for(const char* c = prog; *c != '\0'; c++) {
        if(*c == '\n' && c[1] == '\n') 
            continue;
        switch(state) {
            case StateTitle:
                if(*c == '\n') state++;
            case StateProg:
                *process_p++ = *c;
                break;
            case StateComment:
                if(*c == '\n') state++;
                break;
        }
    }
    *process_p = '\0';
    processed_prog = realloc(processed_prog, strlen(processed_prog)*sizeof(char));
    return processed_prog;
}
