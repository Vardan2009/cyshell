#ifndef CYSH_PARSER_H
#define CYSH_PARSER_H

#include "error.h"
#include "lex.h"
#include "ptr.h"
#include "result.h"
#include "vec.h"

struct cyNode {
    enum type {
        IDENT,
        STRING,
        NUMBER,
        VAR,
        CMD_GROUP,
        CMD,
        BINOP,
        CAPT_CMD,
    };

    type t;
    cyVec<cyPtr<cyNode>> children;

    union {
        struct {
            const char *start;
            size_t len;
        } str;
        cyTok::type tt;
    } val;

    void print(int indent = 0);

    cyNode(type t, size_t initCap = 1) : t(t), children(initCap) {
        val.str.start = NULL;
    }
    cyNode(type t, const char *start, size_t len, size_t initCap = 1)
        : t(t), children(initCap) {
        val.str.start = start;
        val.str.len = len;
    }
    cyNode(type t, cyTok::type tt, size_t initCap = 1)
        : t(t), children(initCap) {
        val.str.start = NULL;
        val.tt = tt;
    }
};

class cyParser {
   public:
    explicit cyParser(cyLex &lex) : lex(lex), current(-1) {
        auto result = lex.nextTok();
        if (result.ok()) current = result.unwrap();
    }

    cyResult<cyPtr<cyNode>, cyErr> parse() { return cmdGroup(); }

   private:
    static int precedence(cyTok::type op) {
        switch (op) {
            case cyTok::type::STAR:
            case cyTok::type::SLASH:
                return 10;
            case cyTok::type::PLUS:
            case cyTok::type::MINUS:
                return 9;
            default:
                return -1;
        }
    }

    static bool isCmdPart(cyTok::type tt) {
        return tt == cyTok::type::IDENT || tt == cyTok::type::STRING ||
               tt == cyTok::type::VARNAME || tt == cyTok::type::LPAREN ||
               tt == cyTok::type::LBRACKET || tt == cyTok::type::AMPPAREN;
    }

    cyResult<cyPtr<cyNode>, cyErr> expr(int minPrec = 0);
    cyResult<cyPtr<cyNode>, cyErr> primary();

    cyResult<cyPtr<cyNode>, cyErr> cmdGroup();
    cyResult<cyPtr<cyNode>, cyErr> cmd();
    cyResult<cyPtr<cyNode>, cyErr> cmdPart();

    cyResult<cyTok, cyErr> advance() {
        cyTok prev = current;
        auto result = lex.nextTok();

        if (!result.ok()) return result.unwrapErr();
        current = result.unwrap();

        return prev;
    }

    cyLex &lex;
    cyTok current;
};

#endif  // CYSH_PARSER_H
