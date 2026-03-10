#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "run.h"

cyLex lex;

volatile sig_atomic_t gotSigInt = 0;

void cySigInt(int sig) {
    (void)sig;
    gotSigInt = 1;
}

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

    if (argc == 1 && strcmp(argv[0], "exit") == 0) {
        signal(SIGINT, SIG_DFL);
        exit(0);
    }  // else
       //    cyRunCommand(argc, argv);

    for (int i = 0; i < argc; ++i) free(argv[i]);
    free(argv);
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

        cyProc(line, read);

        free(line);
    }
}
