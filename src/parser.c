#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "lex.h"

static cyNode *newLeaf(cyNT type, const char *start, size_t len) {
    cyNode *new = (cyNode *)malloc(sizeof(cyNode));
    new->type = type;
    new->childCount = 0;
    new->children = NULL;

    new->start = start;
    new->len = len;

    return new;
}

static cyTok advance(cyParser *parser) {
    cyTok prev = parser->current;
    parser->current = cyLexNextToken(parser->lex);
    return prev;
}

static cyNode *parseExpr(cyParser *parser) {}

static cyNode *parseCommandList(cyParser *parser);

static cyNode *parseCommandPart(cyParser *parser) {
    cyTok tok = parser->current;
    switch (tok.type) {
        case TT_IDENT:
            advance(parser);
            return newLeaf(NT_IDENT, tok.start, tok.len);

        case TT_STRING:
            advance(parser);
            return newLeaf(NT_STRING, tok.start, tok.len);

        case TT_VARNAME:
            advance(parser);
            return newLeaf(NT_VAR, tok.start, tok.len);
        case TT_LPAREN:
            return parseExpr(parser);
        case TT_LBRACKET: {
            advance(parser);
            cyNode *root = parseCommandList(parser);
            if (parser->current.type != TT_RBRACKET) {
                printf("cysh: unterminated `{`\n");
                exit(1);
            }
        }
    }
}

static cyNode *parseCommand(cyParser *parser) {}

static cyNode *parseCommandList(cyParser *parser) {}

void cyParserInit(cyParser *parser, cyLex *lex) {
    parser->lex = lex;
    parser->current = cyLexNextToken(lex);
}

cyNode *cyParse(cyParser *parser) { return parseCommandList(parser); }
