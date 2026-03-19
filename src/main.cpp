#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "parser.h"

volatile sig_atomic_t gotSigInt = 0;

void cySigInt(int sig) {
    (void)sig;
    gotSigInt = 1;
}

void cyProc(const char *src, size_t len) {
    cyLex l(src, len);

    /*while (true) {
        cyResult<cyTok, cyErr> res = l.nextTok();

        if (res.ok()) {
            cyTok tok = res.unwrap();

            printf("TOKEN [TYPE %02d] `%.*s`\n", tok.t, (int)tok.len,
                   tok.start);

            if (tok.t == cyTok::type::EF) break;
        } else {
            cyErr e = res.unwrapErr();
            printerr(e);

            break;
        }
    }*/

    cyParser p(l);

    auto init = p.init();
    if (!init) printerr(init.error());

    auto n = p.parse();
    if (n)
        (*n)->print();
    else
        printerr(n.error());
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

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

        if (read == 1) {
            free(line);
            break;
        }

        if (read == -1) {
            if (errno == EINTR) {
                clearerr(stdin);
                gotSigInt = 0;
                printf("\n");
                free(line);
                continue;
            }

            free(line);
            break;
        }

        cyProc(line, read);

        free(line);
    }
}
