#include "parser.h"

#include <stdio.h>

#include <memory>

#include "error.h"

void cyNode::print(int indent) {
    for (int i = 0; i < indent; ++i) printf("   ");

    if (t == type::BINOP)
        printf("- [%d] OP %d\n", t, val.tt);
    else if (val.str.start == NULL)
        printf("- [%d]\n", t);
    else
        printf("- [%d] \"%.*s\"\n", t, (int)val.str.len, val.str.start);

    for (size_t i = 0; i < children.size(); ++i) children[i]->print(indent + 1);
}

std::expected<cyNode::uptr, cyErr> cyParser::primary() {
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

            auto t = cmd();
            if (!t) return t;

            if (current.t != cyTok::type::RPAREN)
                return std::unexpected(
                    mkerr(cyErr::SYNTAX_ERR, line, "unclosed `(`"));

            r = advance();
            if (!r) return std::unexpected(r.error());

            return std::move(*t);
        }
        case cyTok::type::LSQR:
        case cyTok::type::LBRACKET: {
            cyTok::type opening = current.t;
            cyNode::type t = opening == cyTok::type::LBRACKET
                                 ? cyNode::type::CMD_GROUP
                                 : cyNode::type::SUBSHELL;

            auto r = advance();
            if (!r) return std::unexpected(r.error());

            auto root = cmdGroup();
            if (!root) return root;
            if (current.t != (opening == cyTok::type::LBRACKET
                                  ? cyTok::type::RBRACKET
                                  : cyTok::type::RSQR))
                return std::unexpected(
                    mkerr(cyErr::SYNTAX_ERR, r->line, "unclosed `%c`",
                          opening == cyTok::type::LBRACKET ? '}' : ']'));

            r = advance();
            if (!r) return std::unexpected(r.error());

            if (current.t == cyTok::type::QMARK) {
                r = advance();
                if (!r) return std::unexpected(r.error());

                t = opening == cyTok::type::LBRACKET ? cyNode::type::CMD_GROUP_Q
                                                     : cyNode::type::SUBSHELL_Q;
            }

            (*root)->t = t;

            return std::move(*root);
        }
        case cyTok::type::EF:
            return std::unexpected(
                mkerr(cyErr::INTERNAL_ERR, current.line, "expected primary"));
        default:
            return std::unexpected(
                mkerr(cyErr::INTERNAL_ERR, current.line, "unhandled primary"));
    }
}

std::expected<cyNode::uptr, cyErr> cyParser::expr(int minPrec) {
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

std::expected<cyNode::uptr, cyErr> cyParser::cmdGroup() {
    auto result =
        std::make_unique<cyNode>(cyNode::type::CMD_GROUP, current.line);

    while (isCmdPart(current.t) || current.t == cyTok::type::LBRACKET) {
        auto r = cmd();
        if (!r) return r;
        result->children.push_back(std::move(*r));
    }

    return result;
}

std::expected<cyNode::uptr, cyErr> cyParser::cmd() {
    auto result = std::make_unique<cyNode>(cyNode::type::CMD, current.line);

    if (current.t == cyTok::type::EMARK) {
        auto r = advance();
        if (!r) return std::unexpected(r.error());

        result->t = cyNode::type::EXPR_CMD;

        auto exprr = expr();
        if (!exprr) return exprr;

        result->children.push_back(std::move(std::move(*exprr)));
    } else {
        if (current.t == cyTok::type::AMP) {
            auto r = advance();
            if (!r) return std::unexpected(r.error());

            result->t = cyNode::type::BG_CMD;
        }

        auto r = cmdPart();
        if (!r) return r;

        if ((*r)->t == cyNode::CMD_GROUP || (*r)->t == cyNode::SUBSHELL)
            return std::move(*r);

        result->children.push_back(std::move(*r));

        while (isCmdPart(current.t)) {
            auto r = cmdPart();
            if (!r) return r;
            result->children.push_back(std::move(*r));
        }
    }

    if (current.t == cyTok::type::SEMI) {
        auto r = advance();
        if (!r) return std::unexpected(r.error());
    }

    return result;
}

std::expected<cyNode::uptr, cyErr> cyParser::cmdPart() {
    cyTok tok = current;
    switch (tok.t) {
        case cyTok::type::IDENT: {
            auto r = advance();
            if (!r) return std::unexpected(r.error());

            return std::make_unique<cyNode>(cyNode::type::IDENT, r->line,
                                            tok.start, tok.len);
        }
        case cyTok::type::STRING:
        case cyTok::type::VARNAME:
        case cyTok::type::LPAREN:
        case cyTok::type::CMDPAREN:
        case cyTok::type::LBRACKET:
        case cyTok::type::LSQR:
            return primary();
        default:
            return std::unexpected(mkerr(cyErr::INTERNAL_ERR, tok.line,
                                         "invalid command part of type %d",
                                         tok.t));
    }
}
