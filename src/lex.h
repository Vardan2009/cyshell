#ifndef CYSH_LEX_H
#define CYSH_LEX_H

#include <stddef.h>

#include <stack>

#include "error.h"
#include "result.h"

struct cyTok {
    enum type {
        IDENT,
        STRING,
        NUMBER,

        LPAREN,
        RPAREN,
        LSQR,
        RSQR,
        LBRACKET,
        RBRACKET,

        PIPE,
        AMPPAREN,
        VARNAME,

        PLUS,
        MINUS,
        STAR,
        SLASH,
        SEMI,

        EF
    };

    type t;
    const char *start;
    size_t len;

    int line;

    cyTok(int line, type t = type::EF, const char *start = NULL, size_t len = 0)
        : t(t), start(start), len(len), line(line) {}
};

class cyLex {
   public:
    cyLex(const char *input, size_t inputSz) : input(input), inputSz(inputSz) {
        push(mode::COMMAND);
    }

    ~cyLex();

    cyResult<cyTok, cyErr> nextTok();

   private:
    enum mode { COMMAND, EXPR };

    const char *input;
    size_t inputSz;
    size_t pos = 0;

    int line = 1;

    std::stack<mode> modeStack;

    inline mode cMode() { return modeStack.top(); }
    inline void push(mode m) { modeStack.push(m); }
    inline void pop() { modeStack.pop(); }

    inline bool isOutBounds() { return pos >= inputSz; }

    cyResult<cyTok, cyErr> numberTok();
    cyResult<cyTok, cyErr> oneCharTok();
    cyResult<cyTok, cyErr> oneCharExprTok();
    cyResult<cyTok, cyErr> stringTok();
    cyResult<cyTok, cyErr> varnameTok();
};

#endif  // CYSH_LEX_H
