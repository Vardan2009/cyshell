#ifndef CYSH_PARSER_H
#define CYSH_PARSER_H

#include <memory>
#include <vector>

#include "error.h"
#include "lex.h"

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
    int line;

    union {
        struct {
            const char *start;
            size_t len;
        } str;
        cyTok::type tt;
    } val;

    void print(int indent = 0);

    cyNode(type t, int line, size_t initCap = 1) : t(t), line(line) {
        children.reserve(initCap);
        val.str.start = NULL;
    }

    cyNode(type t, int line, const char *start, size_t len, size_t initCap = 1)
        : t(t), line(line) {
        children.reserve(initCap);
        val.str.start = start;
        val.str.len = len;
    }

    cyNode(type t, int line, cyTok::type tt, size_t initCap = 1)
        : t(t), line(line) {
        children.reserve(initCap);
        val.str.start = NULL;
        val.tt = tt;
    }
};

class cyParser {
   public:
    explicit cyParser(cyLex &lex) : lex(lex), current(-1) {}

    std::expected<void, cyErr> init() {
        auto result = lex.nextTok();
        if (result)
            current = *result;
        else
            return std::unexpected(result.error());

        return {};
    }

    std::expected<cyNode::uptr, cyErr> parse() { return cmdGroup(); }

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

    std::expected<cyNode::uptr, cyErr> expr(int minPrec = 0);
    std::expected<cyNode::uptr, cyErr> primary();

    std::expected<cyNode::uptr, cyErr> cmdGroup();
    std::expected<cyNode::uptr, cyErr> cmd();
    std::expected<cyNode::uptr, cyErr> cmdPart();

    std::expected<cyTok, cyErr> advance() {
        cyTok prev = current;
        auto result = lex.nextTok();

        if (!result) return result;
        current = *result;

        return prev;
    }

    cyLex &lex;
    cyTok current;
};

#endif  // CYSH_PARSER_H
