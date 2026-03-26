#include "parser.h"

#include <stdio.h>

#include <memory>

#include "error.h"

void cyNode::print(int indent) {
    for (int i = 0; i < indent; ++i) printf("   ");

    if (t == type::BINOP)
        printf("- [%d] OP %d", t, val.tt);
    else if (val.str.start == NULL)
        printf("- [%d]", t);
    else
        printf("- [%d] \"%.*s\"", t, (int)val.str.len, val.str.start);

    if (background)
        printf(" BACKGROUND\n");
    else
        printf("\n");

    for (size_t i = 0; i < children.size(); ++i) children[i]->print(indent + 1);
}

cyParser::result cyParser::primary() {
    switch (current.t) {
        case cyTok::type::NUMBER: {
            auto r = std::make_unique<cyNode>(
                cyNode::type::NUMBER, current.line, current.start, current.len);

            auto a = advance();
            if (!a) return std::unexpected(a.error());

            return r;
        }
        case cyTok::type::VARNAME: {
            auto r = std::make_unique<cyNode>(cyNode::type::VAR, current.line,
                                              current.start, current.len);

            auto a = advance();
            if (!a) return std::unexpected(a.error());

            return r;
        }
        case cyTok::type::STRING: {
            auto r = std::make_unique<cyNode>(
                cyNode::type::STRING, current.line, current.start, current.len);

            auto a = advance();
            if (!a) return std::unexpected(a.error());

            return r;
        }
        case cyTok::type::LPAREN: {
            int line = current.line;

            auto r = advance();
            if (!r) return std::unexpected(r.error());

            auto t = expr();
            if (!t) return t;

            if (current.t != cyTok::type::RPAREN)
                return std::unexpected(
                    mkerr(cyErr::SYNTAX_ERR, line, "unclosed `(`"));

            r = advance();
            if (!r) return std::unexpected(r.error());

            return std::move(*t);
        }
        case cyTok::type::CMDPAREN: {
            int line = current.line;
            auto r = advance();
            if (!r) return std::unexpected(r.error());

            auto t = program();
            if (!t) return t;

            if (current.t != cyTok::type::RPAREN)
                return std::unexpected(
                    mkerr(cyErr::SYNTAX_ERR, line, "unclosed `(`"));

            r = advance();
            if (!r) return std::unexpected(r.error());

            auto final =
                std::make_unique<cyNode>(cyNode::CAPT_PROGRAM, line, 1);
            final->children.push_back(std::move(*t));

            return final;
        }
        case cyTok::type::LSQR:
            return subshellQ();
        case cyTok::type::LBRACKET:
            return commandGroupQ();
        case cyTok::type::AMP:
            return statement();
        case cyTok::type::EF:
            return std::unexpected(
                mkerr(cyErr::INTERNAL_ERR, current.line, "expected primary"));
        default:
            return std::unexpected(
                mkerr(cyErr::INTERNAL_ERR, current.line, "unhandled primary"));
    }
}

cyParser::result cyParser::expr(int minPrec) {
    auto leftr = primary();
    if (!leftr) return leftr;

    auto left = std::move(*leftr);

    while (current.t != cyTok::type::EF) {
        int prec = precedence(current.t);
        if (prec < minPrec) break;

        cyTok::type op = current.t;
        auto opr = advance();
        if (!opr) return std::unexpected(opr.error());

        auto right = expr(prec + 1);
        if (!right) return right;

        auto newLeft = std::make_unique<cyNode>(cyNode::type::BINOP, op, 2);
        newLeft->children.push_back(std::move(left));
        newLeft->children.push_back(std::move(*right));

        left = std::move(newLeft);
    }

    return left;
}

cyParser::result cyParser::program() {
    cyNode::uptr result =
        std::make_unique<cyNode>(cyNode::CMD_GROUP, current.line);

    while (current.t != cyTok::RBRACKET && current.t != cyTok::RPAREN &&
           current.t != cyTok::RSQR && current.t != cyTok::EF) {
        auto s = statement();
        if (!s) return s;
        result->children.push_back(std::move(*s));

        if (current.t == cyTok::SEMI) {
            auto r = advance();
            if (!r) return std::unexpected(r.error());
        }
    }

    return result;
}

cyParser::result cyParser::statement() {
    bool background = false;

    if (current.t == cyTok::AMP) {
        background = true;
        auto r = advance();
        if (!r) return std::unexpected(r.error());
    }

    switch (current.t) {
        case cyTok::LBRACKET: {
            auto result = commandGroup();
            if (!result) return result;
            (*result)->background = background;

            if (current.t == cyTok::SEMI) {
                auto r = advance();
                if (!r) return std::unexpected(r.error());
            }

            return result;
        }
        case cyTok::LSQR: {
            auto result = subshell();
            if (!result) return result;
            (*result)->background = background;

            if (current.t == cyTok::SEMI) {
                auto r = advance();
                if (!r) return std::unexpected(r.error());
            }

            return result;
        }
        case cyTok::EMARK: {
            int line = current.line;
            auto r = advance();
            if (!r) return std::unexpected(r.error());

            auto exp = expr();
            if (!exp) return exp;

            auto result = std::make_unique<cyNode>(cyNode::EXPR_STMT, line, 1);

            result->children.push_back(std::move(*exp));
            result->background = background;

            if (current.t == cyTok::SEMI) {
                auto r = advance();
                if (!r) return std::unexpected(r.error());
            }

            return result;
        }
        default:
            break;
    }

    // TODO: handle if and other statements here

    auto fallback = command();
    if (!fallback) return fallback;

    (*fallback)->background = background;

    if (current.t == cyTok::SEMI) {
        auto r = advance();
        if (!r) return std::unexpected(r.error());
    }

    return fallback;
}

cyParser::result cyParser::command() {
    auto start = commandUnit();
    if (!start) return start;

    auto left = std::move(*start);

    bool piped = false;

    while (current.t == cyTok::PIPE) {
        int line = current.line;
        auto a = advance();
        if (!a) return std::unexpected(a.error());

        auto r = commandUnit();
        if (!r) return r;

        auto right = std::move(*r);

        if (piped) {
            left->children.push_back(std::move(right));
        } else {
            piped = true;

            auto newLeft = std::make_unique<cyNode>(cyNode::PIPE, line, 2);
            newLeft->children.push_back(std::move(left));
            newLeft->children.push_back(std::move(right));

            left = std::move(newLeft);
        }
    }

    return left;
}

cyParser::result cyParser::commandUnit() {
    auto result = std::make_unique<cyNode>(cyNode::CMD, current.t);

    auto start = commandPart();
    if (!start) return start;

    result->children.push_back(std::move(*start));

    while (current.t != cyTok::SEMI && current.t != cyTok::RBRACKET &&
           current.t != cyTok::RPAREN && current.t != cyTok::RSQR &&
           current.t != cyTok::EF && current.t != cyTok::CARET &&
           current.t != cyTok::GT && current.t != cyTok::PIPE) {
        auto r = commandPart();
        if (!r) return r;

        result->children.push_back(std::move(*r));
    }

    while (current.t == cyTok::CARET) {
        auto c = advance();
        if (!c) return std::unexpected(c.error());

        int line = current.line;
        auto left = primary();
        if (!left) return left;

        if (current.t != cyTok::GT) {
            return std::unexpected(
                mkerr(cyErr::SYNTAX_ERR, line, "expected > in fd duplication"));
        }

        auto gt = advance();
        if (!gt) return std::unexpected(gt.error());

        auto right = primary();
        if (!right) return right;

        auto fdup = std::make_unique<cyNode>(cyNode::FDUP, line, 2);

        fdup->children.push_back(std::move(*left));
        fdup->children.push_back(std::move(*right));

        result->children.push_back(std::move(fdup));
    }

    if (current.t == cyTok::GT) {
        int line = current.line;

        auto gt = advance();
        if (!gt) return std::unexpected(gt.error());

        auto filename = commandPart();
        if (!filename) return filename;

        auto redir = std::make_unique<cyNode>(cyNode::REDIRECT, line, 1);
        redir->children.push_back(std::move(*filename));

        result->children.push_back(std::move(redir));
    }

    return result;
}

cyParser::result cyParser::commandPart() {
    if (current.t == cyTok::IDENT) {
        int line = current.line;
        const char *start = current.start;
        size_t len = current.len;

        auto r = advance();
        if (!r) return std::unexpected(r.error());

        return std::make_unique<cyNode>(cyNode::IDENT, line, start, len);
    }

    return primary();
}

cyParser::result cyParser::commandGroup() {
    if (current.t != cyTok::LBRACKET)
        return std::unexpected(mkerr(cyErr::INTERNAL_ERR, current.line,
                                     "command group start is not {"));

    auto r = advance();
    if (!r) return std::unexpected(r.error());

    auto result = program();
    if (!result) return result;

    if (current.t != cyTok::RBRACKET)
        return std::unexpected(mkerr(cyErr::SYNTAX_ERR, current.line,
                                     "expected } after command group"));

    r = advance();
    if (!r) return std::unexpected(r.error());

    return result;
}

cyParser::result cyParser::subshell() {
    int line = current.line;
    if (current.t != cyTok::LSQR)
        return std::unexpected(mkerr(cyErr::INTERNAL_ERR, current.line,
                                     "subshell start is not ["));

    auto r = advance();
    if (!r) return std::unexpected(r.error());

    auto result = program();
    if (!result) return result;

    if (current.t != cyTok::RSQR)
        return std::unexpected(mkerr(cyErr::SYNTAX_ERR, current.line,
                                     "expected ] after subshell"));

    r = advance();
    if (!r) return std::unexpected(r.error());

    auto final = std::make_unique<cyNode>(cyNode::SUBSHELL, line, 1);
    final->children.push_back(std::move(*result));

    return final;
}

cyParser::result cyParser::commandGroupQ() {
    auto r = commandGroup();
    if (!r) return r;

    if (current.t != cyTok::QMARK)
        return std::unexpected(
            mkerr(cyErr::SYNTAX_ERR, current.line, "expected ? for { }?"));

    auto a = advance();
    if (!a) return std::unexpected(a.error());

    (*r)->t = cyNode::type::CMD_GROUP_Q;

    return std::move(*r);
}

cyParser::result cyParser::subshellQ() {
    auto r = subshell();
    if (!r) return r;

    if (current.t != cyTok::QMARK)
        return std::unexpected(
            mkerr(cyErr::SYNTAX_ERR, current.line, "expected ? for [ ]?"));

    auto a = advance();
    if (!a) return std::unexpected(a.error());

    (*r)->t = cyNode::type::SUBSHELL_Q;

    return std::move(*r);
}
