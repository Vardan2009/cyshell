#include "lex.h"

#include <ctype.h>

cyLex::~cyLex() {}

std::expected<cyTok, cyErr> cyLex::oneCharTok() {
    int pos = this->pos++;
    cyTok::type tt;

    switch (input[pos]) {
        case '[':
            push(mode::COMMAND);
            tt = cyTok::type::LSQR;
            break;
        case ']':
            pop();
            tt = cyTok::type::RSQR;
            break;
        case '(':
            push(mode::EXPR);
            tt = cyTok::type::LPAREN;
            break;
        case ')':
            pop();
            tt = cyTok::type::RPAREN;
            break;
        case '{':
            push(mode::COMMAND);
            tt = cyTok::type::LBRACKET;
            break;
        case '}':
            pop();
            tt = cyTok::type::RBRACKET;
            break;
        case '|':
            tt = cyTok::type::PIPE;
            break;
        case '@':
            if (input[this->pos] != '(') {
                return std::unexpected(
                    mkerr(cyErr::SYNTAX_ERR, line, "expected ( after @"));
            }
            ++this->pos;
            push(mode::COMMAND);
            tt = cyTok::type::CMDPAREN;
            break;
        case ';':
            if (modeStack.top() == EXPR_CMD) pop();
            tt = cyTok::type::SEMI;
            break;
        case '?':
            tt = cyTok::type::QMARK;
            break;
        case '!':
            push(mode::EXPR_CMD);
            tt = cyTok::type::EMARK;
            break;
        case '&':
            tt = cyTok::type::AMP;
            break;
        case '^':
            tt = cyTok::type::CARET;
            break;
        case '>':
            tt = cyTok::type::GT;
            break;
        case '<':
            tt = cyTok::type::LT;
            break;
        default:
            return std::unexpected(
                mkerr(cyErr::INTERNAL_ERR, line, "unhandled one-char token"));
    }

    return cyTok(line, tt, &input[pos], this->pos - pos);
}

inline static int isOneChar(char c) {
    return c == '[' || c == ']' || c == '(' || c == ')' || c == '{' ||
           c == '}' || c == '|' || c == '@' || c == ';' || c == '?' ||
           c == '&' || c == '!' || c == '^' || c == '>';
}

inline static int isOneCharExpr(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

std::expected<cyTok, cyErr> cyLex::oneCharExprTok() {
    int pos = this->pos++;
    cyTok::type tt;

    switch (input[pos]) {
        case '+':
            tt = cyTok::type::PLUS;
            break;
        case '-':
            tt = cyTok::type::MINUS;
            break;
        case '*':
            tt = cyTok::type::STAR;
            break;
        case '/':
            tt = cyTok::type::SLASH;
            break;
        default:
            return std::unexpected(mkerr(cyErr::INTERNAL_ERR, line,
                                         "unhandled one-char token in expr"));
    }

    return cyTok(line, tt, &input[pos], this->pos - pos);
}

std::expected<cyTok, cyErr> cyLex::stringTok() {
    int start = ++pos;
    int startl = line;

    while (!isOutBounds() && input[pos] != '"') ++pos;

    if (isOutBounds()) {
        return std::unexpected(
            mkerr(cyErr::SYNTAX_ERR, line, "unterminated string literal"));
    } else {
        int len = pos - start;
        ++pos;

        return cyTok(startl, cyTok::type::STRING, &input[start], len);
    }
}

std::expected<cyTok, cyErr> cyLex::varnameTok() {
    int startl = line;
    ++pos;
    if (isOutBounds()) {
        return std::unexpected(
            mkerr(cyErr::SYNTAX_ERR, line, "expected varname after $"));
    }

    int start = pos;

    while (!isOutBounds() && (isalnum(input[pos]) || input[pos] == '_')) ++pos;

    return cyTok(startl, cyTok::type::VARNAME, &input[start], pos - start);
}

std::expected<cyTok, cyErr> cyLex::numberTok() {
    int start = pos;
    int startl = line;
    bool decPnt = false;
    while (!isOutBounds() &&
           (isdigit(input[pos]) || (!decPnt && input[pos == '.']))) {
        if (input[pos == '.']) decPnt = true;
        ++pos;
    }

    return cyTok(startl, cyTok::type::NUMBER, &input[start], pos - start);
}

std::expected<cyTok, cyErr> cyLex::nextTok() {
    while (true) {
        while (isspace(input[pos])) {
            if (input[pos] == '\n') ++line;
            ++pos;
        }

        if (input[pos] == '#') {
            while (input[pos] && input[pos] != '\n') ++pos;
            if (input[pos] == '\n') ++line;

            continue;
        }

        break;
    }

    if (isOutBounds()) return cyTok(cyTok::type::EF);

    char c = input[pos];

    if (isOneChar(c)) return oneCharTok();
    if (c == '"') return stringTok();

    if (c == '$') return varnameTok();

    switch (cMode()) {
        case mode::EXPR:
        case mode::EXPR_CMD: {
            if (isdigit(c)) return numberTok();
            if (isOneCharExpr(c)) return oneCharExprTok();

            return std::unexpected(
                mkerr(cyErr::SYNTAX_ERR, line, "unhandled character `%c`", c));
        }
        case mode::COMMAND: {
            int start = pos;
            int startl = line;

            while (!isspace(c) && !isOneChar(c)) {
                ++pos;
                c = input[pos];

                if (isOutBounds()) break;
            }

            return cyTok(startl, cyTok::type::IDENT, &input[start],
                         pos - start);
        }
        default:
            return std::unexpected(
                mkerr(cyErr::INTERNAL_ERR, line, "unhandled lexer mode"));
    }
}
