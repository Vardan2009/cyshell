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
        CMD_GROUP_Q,
        SUBSHELL,
        SUBSHELL_Q,
        CMD,
        BINOP,
        CAPT_PROGRAM,
        EXPR_STMT,
        PIPE,

        FDUP,
        REDIRECT
    };

    type t;
    std::vector<uptr> children;
    int line;

    bool background = false;

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

    using result = std::expected<cyNode::uptr, cyErr>;

    std::expected<void, cyErr> init() {
        auto result = lex.nextTok();
        if (result)
            current = *result;
        else
            return std::unexpected(result.error());

        return {};
    }

    result parse() { return program(); }

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

    result expr(int minPrec = 0);
    result primary();

    result program();
    result statement();
    result command();
    result commandUnit();
    result commandPart();

    result commandGroup();
    result commandGroupQ();

    result subshell();
    result subshellQ();

    result commandCapture();

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
