#ifndef CYSH_ERROR_H
#define CYSH_ERROR_H

struct cyErr {
    enum code {
        UNKNOWN_ERR,
        INTERNAL_ERR,
        SYNTAX_ERR,
    };

    code errcode;
    char message[256];

    int line;
};

cyErr mkerr(cyErr::code code, int line, const char *fmt, ...);

void printerr(const cyErr &e);

#endif  // CYSH_ERROR_H
