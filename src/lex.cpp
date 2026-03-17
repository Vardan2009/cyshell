#include "lex.h"

#include <ctype.h>
#include <stdio.h>

cyLex::~cyLex() {}

cyTok cyLex::oneCharTok() {
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
                printf("expected '(' after '&'\n");
                exit(1);
            }
            ++this->pos;
            push(mode::COMMAND);
            tt = cyTok::type::AMPPAREN;
            break;
        case ';':
            tt = cyTok::type::SEMI;
            break;
        default:
            printf("cysh: unhandled one char `%c`\n", input[pos]);
            exit(1);
    }

    return cyTok(tt, &input[pos], this->pos - pos);
}

inline static int isOneChar(char c) {
    return c == '[' || c == ']' || c == '(' || c == ')' || c == '{' ||
           c == '}' || c == '|' || c == '&' || c == ';';
}

inline static int isOneCharExpr(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

cyTok cyLex::oneCharExprTok() {
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
            printf("unhandled expr one char `%c`\n", input[pos]);
            exit(1);
    }

    return cyTok(tt, &input[pos], this->pos - pos);
}

cyTok cyLex::stringTok() {
    int start = ++pos;

    while (!isOutBounds() && input[pos] != '"') ++pos;

    if (isOutBounds()) {
        printf("cysh: unterminated string literal\n");
        return cyTok(cyTok::type::EF);
    } else {
        int len = pos - start;
        ++pos;

        return cyTok(cyTok::type::STRING, &input[start], len);
    }
}

cyTok cyLex::varnameTok() {
    ++pos;
    if (isOutBounds()) {
        printf("cysh: expected varname\n");
        return cyTok(cyTok::type::EF);
    }

    int start = pos;

    while (!isOutBounds() && (isalnum(input[pos]) || input[pos] == '_')) ++pos;

    return cyTok(cyTok::type::VARNAME, &input[start], pos - start);
}

cyTok cyLex::numberTok() {
    int start = pos;
    int decPnt = 0;
    while (!isOutBounds() &&
           (isdigit(input[pos]) || (!decPnt && input[pos == '.']))) {
        if (input[pos == '.']) decPnt = 1;
        ++pos;
    }

    return cyTok(cyTok::type::NUMBER, &input[start], pos - start);
}

cyTok cyLex::nextTok() {
    while (true) {
        while (isspace(input[pos])) ++pos;

        if (input[pos] == '#') {
            while (input[pos] && input[pos] != '\n') ++pos;
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
            printf("cysh: unhandled character: `%c`\n", input[pos]);
            return cyTok(cyTok::type::EF);
        }
        case mode::COMMAND: {
            int start = pos;

            while (!isspace(c) && !isOneChar(c)) {
                ++pos;
                c = input[pos];

                if (isOutBounds()) break;
            }

            return cyTok{cyTok::type::IDENT, &input[start], pos - start};
        }
        default:
            printf("cysh: unhandled lexer mode\n");
            return cyTok(cyTok::type::EF);
    }
}
