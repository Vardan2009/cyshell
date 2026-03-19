#include "error.h"

#include <stdarg.h>
#include <stdio.h>

cyErr mkerr(cyErr::code code, int line, const char *fmt, ...) {
    cyErr err;
    err.errcode = code;

    err.line = line;

    va_list args;
    va_start(args, fmt);
    vsnprintf(err.message, sizeof(err.message), fmt, args);
    va_end(args);

    return err;
}

const static char *codeStr[] = {"UNKNOWN_ERR", "INTERNAL_ERR", "SYNTAX_ERR",
                                "TYPE_ERR"};

void printerr(const cyErr &e) {
    fprintf(stderr, "cysh: %s at line %d: %s\n", codeStr[(int)e.errcode],
            e.line, e.message);
}
