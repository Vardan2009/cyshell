#include "lex.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void cyLexInit(cyLex *lex, const char *src, size_t len) {
    lex->input = src;
    lex->inputSz = len;
    lex->pos = 0;

    // I don't think allocating/deallocating this for each command
    // is a good idea, might change this later.
    const size_t initCap = 512;
    lex->modeStack = malloc(initCap * sizeof(cyLexMode));
    lex->modeStackCap = initCap;
    lex->modeStackTop = 1;
    lex->modeStack[0] = M_COMMAND;
}

inline static cyLexMode lexMode(cyLex *lex) {
    return lex->modeStack[lex->modeStackTop - 1];
}

inline static void lexPush(cyLex *lex, cyLexMode mode) {
    if (lex->modeStackCap >= lex->modeStackTop) {
        lex->modeStackCap += 256;
        lex->modeStack =
            realloc(lex->modeStack, lex->modeStackCap * sizeof(cyLexMode));
    }

    lex->modeStack[lex->modeStackTop++] = mode;
}

inline static void lexPop(cyLex *lex) { --lex->modeStackTop; }

void cyLexFree(cyLex *lex) { free(lex->modeStack); }

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
            lexPush(lex, M_EXPR);
            tt = TT_LPAREN;
            break;
        case ')':
            lexPop(lex);
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
        case ';':
            tt = TT_SEMI;
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
           c == '}' || c == '|' || c == '&' || c == ';';
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
    while (1) {
        while (isspace(lex->input[lex->pos])) ++lex->pos;

        if (lex->input[lex->pos] == '#') {
            while (lex->input[lex->pos] && lex->input[lex->pos] != '\n')
                ++lex->pos;
            continue;
        }

        break;
    }

    if (isOutBounds(lex)) return (cyTok){TT_EOF};

    char c = lex->input[lex->pos];

    if (isOneChar(c)) return oneCharTok(lex);
    if (c == '"') return stringTok(lex);

    if (c == '$') return varnameTok(lex);

    switch (lexMode(lex)) {
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
