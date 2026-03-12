#include "lex.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void cyLexInit(cyLex *lex, const char *src, size_t len) {
    lex->input = src;
    lex->inputSz = len;
    lex->pos = 0;
    lex->mode = M_COMMAND;
    lex->parenDepth = 0;
}

static cyTok oneCharTok(cyLex *lex) {
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
            lex->mode = M_EXPR;
            ++lex->parenDepth;
            tt = TT_LPAREN;
            break;
        case ')':
            if (--lex->parenDepth <= 0) {
                lex->parenDepth = 0;
                lex->mode = M_COMMAND;
            }
            tt = TT_RPAREN;
            break;
        case '{':
            tt = TT_LBRACKET;
            break;
        case '}':
            tt = TT_RBRACKET;
            break;
        case '|':
            tt = TT_PIPE;
            break;
        case '&':
            tt = TT_AMP;
            break;
        default:
            printf("unhandled one char `%c`\n", lex->input[pos]);
            exit(1);
    }

    return (cyTok){tt, &lex->input[pos], 1};
}

static cyTok oneCharExprTok(cyLex *lex) {
    int pos = lex->pos++;
    cyTT tt;

    switch (lex->input[pos]) {
        case '+':
            tt = TT_PLUS;
            break;
        case '-':
            tt = TT_MINUS;
            break;
        case '*':
            tt = TT_STAR;
            break;
        case '/':
            tt = TT_SLASH;
            break;
        default:
            printf("unhandled expr one char `%c`\n", lex->input[pos]);
            exit(1);
    }

    return (cyTok){tt, &lex->input[pos], 1};
}

inline static int isOneChar(char c) {
    return c == '[' || c == ']' || c == '(' || c == ')' || c == '{' ||
           c == '}' || c == '|' || c == '&';
}

inline static int isOneCharExpr(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

inline static int isOutBounds(cyLex *lex) { return lex->pos >= lex->inputSz; }

static cyTok stringTok(cyLex *lex) {
    int start = ++lex->pos;

    while (!isOutBounds(lex) && lex->input[lex->pos] != '"') ++lex->pos;

    if (isOutBounds(lex)) {
        printf("cysh: unterminated string literal\n");
        return (cyTok){TT_EOF};
    } else {
        int len = lex->pos - start;
        ++lex->pos;

        return (cyTok){TT_STRING, &lex->input[start], len};
    }
}

static cyTok varnameTok(cyLex *lex) {
    ++lex->pos;
    if (isOutBounds(lex)) {
        printf("cysh: expected varname\n");
        return (cyTok){TT_EOF};
    }

    int start = lex->pos;

    while (!isOutBounds(lex) &&
           (isalnum(lex->input[lex->pos]) || lex->input[lex->pos] == '_'))
        ++lex->pos;

    return (cyTok){TT_VARNAME, &lex->input[start], lex->pos - start};
}

static cyTok numberTok(cyLex *lex) {
    int start = lex->pos;
    int decPnt = 0;
    while (!isOutBounds(lex) && (isdigit(lex->input[lex->pos]) ||
                                 (!decPnt && lex->input[lex->pos == '.']))) {
        if (lex->input[lex->pos == '.']) decPnt = 1;
        ++lex->pos;
    }

    return (cyTok){TT_NUMBER, &lex->input[start], lex->pos - start};
}

cyTok cyLexNextToken(cyLex *lex) {
    while (isspace(lex->input[lex->pos])) ++lex->pos;

    if (isOutBounds(lex)) return (cyTok){TT_EOF};

    char c = lex->input[lex->pos];

    if (isOneChar(c)) return oneCharTok(lex);
    if (c == '"') return stringTok(lex);

    if (c == '$') return varnameTok(lex);

    switch (lex->mode) {
        case M_EXPR: {
            if (isdigit(c)) return numberTok(lex);
            if (isOneCharExpr(c)) return oneCharExprTok(lex);
            printf("cysh: unhandled character: `%c`\n", lex->input[lex->pos]);
            return (cyTok){TT_EOF};
        }
        case M_COMMAND: {
            int start = lex->pos;

            while (!isspace(c) && !isOneChar(c)) {
                ++lex->pos;
                c = lex->input[lex->pos];

                if (isOutBounds(lex)) break;
            }

            return (cyTok){TT_IDENT, &lex->input[start], lex->pos - start};
        }
    }
}
