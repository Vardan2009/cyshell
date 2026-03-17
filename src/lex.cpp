#include "lex.h"

#include <ctype.h>
#include <stdio.h>

#include "error.h"
#include "result.h"

cyLex::~cyLex() {}

cyResult<cyTok, cyErr> cyLex::oneCharTok() {
    int pos = this->pos++;
    cyTok::type tt;

    switch (input[pos]) {
        case '[':
            tt = cyTok::type::LSQR;
            break;
        case ']':
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
            tt = cyTok::type::LBRACKET;
            break;
        case '}':
            tt = cyTok::type::RBRACKET;
            break;
        case '|':
            tt = cyTok::type::PIPE;
            break;
        case '&':
            if (input[this->pos] != '(') {
                return resErr<cyTok, cyErr>(
                    mkerr(cyErr::SYNTAX_ERR, line, "expected ( after &"));
            }
            ++this->pos;
            push(mode::COMMAND);
            tt = cyTok::type::AMPPAREN;
            break;
        case ';':
            tt = cyTok::type::SEMI;
            break;
        default:
            return resErr<cyTok, cyErr>(
                mkerr(cyErr::INTERNAL_ERR, line, "unhandled one-char token"));
    }

    return resOk<cyTok, cyErr>(cyTok(tt, &input[pos], this->pos - pos));
}

inline static int isOneChar(char c) {
    return c == '[' || c == ']' || c == '(' || c == ')' || c == '{' ||
           c == '}' || c == '|' || c == '&' || c == ';';
}

inline static int isOneCharExpr(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

cyResult<cyTok, cyErr> cyLex::oneCharExprTok() {
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
            return resErr<cyTok, cyErr>(mkerr(
                cyErr::INTERNAL_ERR, line, "unhandled one-char token in expr"));
    }

    return resOk<cyTok, cyErr>(cyTok(tt, &input[pos], this->pos - pos));
}

cyResult<cyTok, cyErr> cyLex::stringTok() {
    int start = ++pos;

    while (!isOutBounds() && input[pos] != '"') ++pos;

    if (isOutBounds()) {
        return resErr<cyTok, cyErr>(
            mkerr(cyErr::SYNTAX_ERR, line, "unterminated string literal"));
    } else {
        int len = pos - start;
        ++pos;

        return resOk<cyTok, cyErr>(
            cyTok(cyTok::type::STRING, &input[start], len));
    }
}

cyResult<cyTok, cyErr> cyLex::varnameTok() {
    ++pos;
    if (isOutBounds()) {
        return resErr<cyTok, cyErr>(
            mkerr(cyErr::SYNTAX_ERR, line, "expected varname after $"));
    }

    int start = pos;

    while (!isOutBounds() && (isalnum(input[pos]) || input[pos] == '_')) ++pos;

    return resOk<cyTok, cyErr>(
        cyTok(cyTok::type::VARNAME, &input[start], pos - start));
}

cyResult<cyTok, cyErr> cyLex::numberTok() {
    int start = pos;
    int decPnt = 0;
    while (!isOutBounds() &&
           (isdigit(input[pos]) || (!decPnt && input[pos == '.']))) {
        if (input[pos == '.']) decPnt = 1;
        ++pos;
    }

    return resOk<cyTok, cyErr>(
        cyTok(cyTok::type::NUMBER, &input[start], pos - start));
}

cyResult<cyTok, cyErr> cyLex::nextTok() {
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
        case mode::EXPR: {
            if (isdigit(c)) return numberTok();
            if (isOneCharExpr(c)) return oneCharExprTok();

            return resErr<cyTok, cyErr>(
                mkerr(cyErr::SYNTAX_ERR, line, "unhandled character `%c`", c));
        }
        case mode::COMMAND: {
            int start = pos;

            while (!isspace(c) && !isOneChar(c)) {
                ++pos;
                c = input[pos];

                if (isOutBounds()) break;
            }

            return resOk<cyTok, cyErr>(
                cyTok(cyTok::type::IDENT, &input[start], pos - start));
        }
        default:
            return resErr<cyTok, cyErr>(
                mkerr(cyErr::INTERNAL_ERR, line, "unhandled lexer mode"));
    }
}
