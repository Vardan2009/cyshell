#ifndef CYSH_PARSER_H
#define CYSH_PARSER_H

#include <memory>
#include <vector>

#include "error.h"
#include "lex.h"
#include "result.h"

struct cyNode {
    using uptr = std::unique_ptr<cyNode>;

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
    std::vector<uptr> children;

    union {
        struct {
            const char *start;
            size_t len;
        } str;
        cyTok::type tt;
    } val;

    void print(int indent = 0);

    cyNode(type t, size_t initCap = 1) : t(t) {
        children.reserve(initCap);
        val.str.start = NULL;
    }

    cyNode(type t, const char *start, size_t len, size_t initCap = 1) : t(t) {
        children.reserve(initCap);
        val.str.start = start;
        val.str.len = len;
    }

    cyNode(type t, cyTok::type tt, size_t initCap = 1) : t(t) {
        children.reserve(initCap);
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

    cyResult<cyNode::uptr, cyErr> parse() { return cmdGroup(); }

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

    cyResult<cyNode::uptr, cyErr> expr(int minPrec = 0);
    cyResult<cyNode::uptr, cyErr> primary();

    cyResult<cyNode::uptr, cyErr> cmdGroup();
    cyResult<cyNode::uptr, cyErr> cmd();
    cyResult<cyNode::uptr, cyErr> cmdPart();

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
