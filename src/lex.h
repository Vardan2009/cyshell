#ifndef CYSH_LEX_H
#define CYSH_LEX_H

#include <stddef.h>

#include <expected>
#include <stack>

#include "error.h"

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
        CMDPAREN,
        VARNAME,
        QMARK,
        EMARK,
        AMP,

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

    std::expected<cyTok, cyErr> nextTok();

   private:
    enum mode { COMMAND, EXPR, EXPR_CMD };

    const char *input;
    size_t inputSz;
    size_t pos = 0;

    int line = 1;

    std::stack<mode> modeStack;

    inline mode cMode() { return modeStack.top(); }
    inline void push(mode m) { modeStack.push(m); }
    inline void pop() { modeStack.pop(); }

    inline bool isOutBounds() { return pos >= inputSz; }

    std::expected<cyTok, cyErr> numberTok();
    std::expected<cyTok, cyErr> oneCharTok();
    std::expected<cyTok, cyErr> oneCharExprTok();
    std::expected<cyTok, cyErr> stringTok();
    std::expected<cyTok, cyErr> varnameTok();
};

#endif  // CYSH_LEX_H
