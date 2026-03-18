#include "parser.h"

#include <stdio.h>

#include "error.h"
#include "result.h"

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

cyResult<cyPtr<cyNode>, cyErr> cyParser::primary() {
    switch (current.t) {
        case cyTok::type::NUMBER: {
            auto r =
                mkptr<cyNode>(cyNode::type::NUMBER, current.start, current.len);

            auto a = advance();
            if (!a.ok()) return a.unwrapErr();

            return r;
        }
        case cyTok::type::VARNAME: {
            auto r =
                mkptr<cyNode>(cyNode::type::VAR, current.start, current.len);

            auto a = advance();
            if (!a.ok()) return a.unwrapErr();

            return r;
        }
        case cyTok::type::STRING: {
            auto r =
                mkptr<cyNode>(cyNode::type::STRING, current.start, current.len);

            auto a = advance();
            if (!a.ok()) return a.unwrapErr();

            return r;
        }
        case cyTok::type::LPAREN: {
            int line = current.line;

            auto r = advance();
            if (!r.ok()) return r.unwrapErr();

            auto t = expr();
            if (!t.ok()) return t.unwrapErr();
            if (current.t != cyTok::type::RPAREN)
                return mkerr(cyErr::SYNTAX_ERR, line, "unclosed `(`");

            r = advance();
            if (!r.ok()) return r.unwrapErr();

            return t.unwrap();
        }
        case cyTok::type::AMPPAREN: {
            int line = current.line;
            auto r = advance();
            if (!r.ok()) return r.unwrapErr();

            auto t = cmd();
            if (!t.ok()) return t.unwrapErr();

            if (current.t != cyTok::type::RPAREN)
                return mkerr(cyErr::SYNTAX_ERR, line, "unclosed `(`");

            r = advance();
            if (!r.ok()) return r.unwrapErr();

            return t.unwrap();
        }
        default:
            return mkerr(cyErr::INTERNAL_ERR, current.line,
                         "unhandled primary");
    }
}

cyResult<cyPtr<cyNode>, cyErr> cyParser::expr(int minPrec) {
    auto leftr = primary();
    if (!leftr.ok()) return leftr.unwrapErr();
    auto left = leftr.unwrap();

    while (current.t != cyTok::type::EF) {
        int prec = precedence(current.t);
        if (prec < minPrec) break;

        cyTok::type op = current.t;
        auto opr = advance();
        if (!opr.ok()) return opr.unwrapErr();

        auto right = expr(prec + 1);
        if (!right.ok()) return right.unwrapErr();

        auto newLeft = mkptr<cyNode>(cyNode::type::BINOP, op, 2);
        newLeft->children.push(left);
        newLeft->children.push(right.unwrap());

        left = newLeft;
    }

    return left;
}

cyResult<cyPtr<cyNode>, cyErr> cyParser::cmdGroup() {
    auto result = mkptr<cyNode>(cyNode::type::CMD_GROUP);

    while (isCmdPart(current.t)) {
        auto r = cmd();
        if (!r.ok()) return r.unwrapErr();
        result->children.push(r.unwrap());
    }

    return result;
}

cyResult<cyPtr<cyNode>, cyErr> cyParser::cmd() {
    auto result = mkptr<cyNode>(cyNode::type::CMD);

    while (isCmdPart(current.t)) {
        auto r = cmdPart();
        if (!r.ok()) return r.unwrapErr();
        result->children.push(r.unwrap());
    }

    if (current.t == cyTok::type::SEMI) {
        auto r = advance();
        if (!r.ok()) return r.unwrapErr();
    }

    return result;
}

cyResult<cyPtr<cyNode>, cyErr> cyParser::cmdPart() {
    cyTok tok = current;
    switch (tok.t) {
        case cyTok::type::IDENT: {
            auto r = advance();
            if (!r.ok()) return r.unwrapErr();

            return mkptr<cyNode>(cyNode::type::IDENT, tok.start, tok.len);
        }
        case cyTok::type::STRING:
        case cyTok::type::VARNAME:
        case cyTok::type::LPAREN:
        case cyTok::type::AMPPAREN:
            return primary();
        case cyTok::type::LBRACKET: {
            auto r = advance();
            if (!r.ok()) return r.unwrapErr();

            auto root = cmdGroup();
            if (!root.ok()) return root.unwrapErr();
            if (current.t != cyTok::type::RBRACKET)
                return mkerr(cyErr::SYNTAX_ERR, r.unwrap().line,
                             "unclosed `{`");

            r = advance();
            if (!r.ok()) return r.unwrapErr();

            return root.unwrap();
        }
        default:
            return mkerr(cyErr::INTERNAL_ERR, tok.line,
                         "invalid command part of type %d", tok.t);
    }
}
