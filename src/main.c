#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "run.h"

cyLex lex;

void cyProc(const char *src, size_t len) {
    cyLexInit(&lex, src, len);

    cyTok tok;

    char **argv = NULL;
    int argc = 0;

    while ((tok = cyLexNextToken(&lex)).type != TT_EOF) {
        printf("TOKEN [TYPE %02d] `%.*s`\n", tok.type, (int)tok.len, tok.start);

        char *str = malloc(tok.len + 1);
        strncpy(str, tok.start, tok.len);
        str[tok.len] = 0;

        ++argc;
        argv = realloc(argv, sizeof(char *) * argc);

        argv[argc - 1] = str;
    }

    cyRunCommand(argc, argv);

    for (int i = 0; i < argc; ++i) free(argv[i]);
    free(argv);
}

int main(int argc, char *argv[]) {
    printf("cyShell\n\n");

    while (1) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        printf("$ ");

        read = getline(&line, &len, stdin);

        if (read == -1) {
            free(line);
            continue;
        }

        cyProc(line, read);

        free(line);
    }
}
