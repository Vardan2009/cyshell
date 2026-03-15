#ifndef _CYSH_LEX
#define _CYSH_LEX

#include <stddef.h>

typedef enum { M_COMMAND, M_EXPR } cyLexMode;

typedef struct {
    const char *input;
    size_t inputSz;
    int pos;

    cyLexMode *modeStack;
    size_t modeStackCap;
    size_t modeStackTop;
} cyLex;

typedef enum {
    TT_IDENT,
    TT_STRING,
    TT_NUMBER,

    TT_LPAREN,
    TT_RPAREN,
    TT_LSQR,
    TT_RSQR,
    TT_LBRACKET,
    TT_RBRACKET,

    TT_PIPE,
    TT_AMP,
    TT_VARNAME,

    TT_PLUS,
    TT_MINUS,
    TT_STAR,
    TT_SLASH,
    TT_SEMI,

    TT_EOF
} cyTT;

typedef struct {
    cyTT type;
    const char *start;
    size_t len;
} cyTok;

void cyLexInit(cyLex *lex, const char *src, size_t len);
void cyLexFree(cyLex *lex);

cyTok cyLexNextToken(cyLex *lex);

#endif  // _CYSH_LEX
