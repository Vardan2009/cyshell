#include <stdio.h>
#include <stdlib.h>

#include "lex.h"

cyLex lex;

void cyProc(const char *src, size_t len) {
    cyLexInit(&lex, src, len);

    cyTok tok;

    while ((tok = cyLexNextToken(&lex)).type != TT_EOF)
        printf("TOKEN [TYPE %02d] `%.*s`\n", tok.type, (int)tok.len, tok.start);
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
