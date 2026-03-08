#include "lex.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void cyLexInit(cyLex *lex, const char *src, size_t len) {
    lex->input = src;
    lex->inputSz = len;
    lex->pos = 0;
    lex->mode = M_COMMAND;
}

cyTok oneCharTok(cyLex *lex) {
    int pos = lex->pos++;
    cyTT tt;

    switch (lex->input[pos]) {
        case '[':
            tt = TT_LSQR;
            break;
        case ']':
            tt = TT_RSQR;
            break;
        case '(':
            tt = TT_LPAREN;
            break;
        case ')':
            tt = TT_RPAREN;
            break;
        case '{':
            tt = TT_LBRACKET;
            break;
        case '}':
            tt = TT_RBRACKET;
            break;
        default:
            printf("unhandled one char `%c`\n", lex->input[pos]);
            exit(1);
    }

    return (cyTok){tt, &lex->input[pos], 1};
}

inline static int isOneChar(char c) {
    return c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}';
}

cyTok cyLexNextToken(cyLex *lex) {
    while (isspace(lex->input[lex->pos])) ++lex->pos;

    if (lex->pos >= lex->inputSz) return (cyTok){TT_EOF};

    char c = lex->input[lex->pos];

    if (isOneChar(c)) return oneCharTok(lex);

    switch (lex->mode) {
        case M_EXPR:
            printf("expr mode\n");
            exit(1);
        case M_COMMAND: {
            int start = lex->pos;

            while (!isspace(c) && !isOneChar(c)) {
                ++lex->pos;
                c = lex->input[lex->pos];

                if (lex->pos >= lex->inputSz) break;
            }

            return (cyTok){TT_IDENT, &lex->input[start], lex->pos - start};
        }
    }
}
