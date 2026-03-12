#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "parser.h"
#include "run.h"

cyLex lex;
cyParser parser;

volatile sig_atomic_t gotSigInt = 0;

void cySigInt(int sig) {
    (void)sig;
    gotSigInt = 1;
}

void cyProc(const char *src, size_t len) {
    cyLexInit(&lex, src, len);
    cyParserInit(&parser, &lex);

    cyNode *root = cyParse(&parser);
    cyNodePrint(root, 0);
    cyNodeFree(root);

    // cyTok tok;
    // while ((tok = cyLexNextToken(&lex)).type != TT_EOF)
    //    printf("TOKEN [TYPE %02d] `%.*s`\n", tok.type, (int)tok.len,
    //    tok.start);
}

int main(int argc, char *argv[]) {
    printf("cyShell\n\n");

    struct sigaction sa;
    sa.sa_handler = cySigInt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    while (1) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        printf("$ ");

        read = getline(&line, &len, stdin);

        if (read == -1) {
            if (errno == EINTR) {
                clearerr(stdin);
                gotSigInt = 0;
                printf("\n");
                continue;
            }

            free(line);
            break;
        }

        if (read == 1) break;

        cyProc(line, read);

        free(line);
    }
}
