#include "run.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void cyRunCommand(int argc, char **argv) {
    if (argc == 0) return;

    pid_t pid = fork();

    char **argvnull = malloc((argc + 1) * sizeof(char *));

    for (int i = 0; i < argc; ++i) argvnull[i] = argv[i];
    argvnull[argc] = NULL;

    if (pid == 0) {
        execv(argv[0], argvnull);
        perror("execv");
    }

    free(argvnull);
    wait(NULL);
}
