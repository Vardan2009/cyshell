#include "run.h"

#include <sstream>

cyScope::sptr cyScope::createGlobal() {
    // TODO: add environment variables (as strings)
    return std::make_unique<cyScope>();
}

std::expected<double, cyErr> parseNumber(const std::string &strnum, int line) {
    std::stringstream ss(strnum);

    double result;
    if (ss >> result) return result;

    return std::unexpected(
        mkerr(cyErr::SYNTAX_ERR, line, "`%s` is not a number", strnum.c_str()));
}

static evalRes evalCmdGroup(cyNode::uptr node, cyScope::sptr scope) {
    return std::unexpected(mkerr(cyErr::INTERNAL_ERR, node->line,
                                 "evalCmdGroup() not implemented"));
}

static evalRes evalCmd(cyNode::uptr node, cyScope::sptr scope,
                       bool captured = false) {
    return std::unexpected(
        mkerr(cyErr::INTERNAL_ERR, node->line, "evalCmd() not implemented"));
}

static evalRes evalBinOp(cyNode::uptr node, cyScope::sptr scope) {
    return std::unexpected(
        mkerr(cyErr::INTERNAL_ERR, node->line, "evalBinOp() not implemented"));
}

evalRes eval(cyNode::uptr node, cyScope::sptr scope) {
    switch (node->t) {
        case cyNode::STRING:
            return cyVal(std::string(node->val.str.start, node->val.str.len));
        case cyNode::NUMBER: {
            auto r =
                parseNumber(std::string(node->val.str.start, node->val.str.len),
                            node->line);

            if (!r) return std::unexpected(r.error());

            return cyVal(*r);
        }
        case cyNode::VAR: {
            std::string name(node->val.str.start, node->val.str.len);

            auto r = scope->find(name, node->line);
            if (!r) return std::unexpected(r.error());

            return (*r).get().val;
        }
        case cyNode::CMD_GROUP:
            return evalCmdGroup(std::move(node), scope);
        case cyNode::CMD:
            return evalCmd(std::move(node), scope);
        case cyNode::BINOP:
            return evalBinOp(std::move(node), scope);
        case cyNode::CAPT_CMD:
            return evalCmd(std::move(node), scope, true);
        default:
            return std::unexpected(
                mkerr(cyErr::INTERNAL_ERR, node->line, "unhandled node"));
    }
}
