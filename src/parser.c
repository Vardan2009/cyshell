#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "lex.h"

static cyNode *newLeaf(cyNT type, const char *start, size_t len) {
    cyNode *new = (cyNode *)malloc(sizeof(cyNode));
    new->type = type;
    new->list = (cyNodeList){NULL, 0, 0};
    new->start = start;
    new->len = len;

    return new;
}

inline static void listFreeItems(cyNodeList *lst);

void cyNodeFree(cyNode *node) {
    for (int i = 0; i < node->list.count; ++i) cyNodeFree(node->list.data[i]);
    listFreeItems(&node->list);
}

void cyNodePrint(cyNode *node, int indent) {
    for (int i = 0; i < indent; ++i) printf("   ");

    if (node->start == NULL)
        printf("- [%d]\n", node->type);
    else
        printf("- [%d] \"%.*s\"\n", node->type, (int)node->len, node->start);

    for (int i = 0; i < node->list.count; ++i)
        cyNodePrint(node->list.data[i], indent + 1);
}

static cyNodeList newList(size_t initialSz) {
    cyNodeList result;
    result.data = (cyNode **)malloc(sizeof(cyNode *) * initialSz);
    result.capacity = (initialSz > 0) ? initialSz : 1;
    result.count = initialSz;
    return result;
}

inline static void listReserve(cyNodeList *lst, size_t delta) {
    lst->capacity += delta;
    lst->data = realloc(lst->data, (lst->capacity + delta) * sizeof(cyNode *));
}

inline static void listPush(cyNodeList *lst, cyNode *item) {
    if (lst->count >= lst->capacity) listReserve(lst, 1);
    lst->data[lst->count++] = item;
}

inline static void listFreeItems(cyNodeList *lst) {
    for (int i = 0; i < lst->count; ++i) free(lst->data[i]);
}

static cyTok advance(cyParser *parser) {
    cyTok prev = parser->current;
    parser->current = cyLexNextToken(parser->lex);
    return prev;
}

static cyNode *parseExpr(cyParser *parser) {
    // TODO: Finish this
}

static cyNode *parseCommandList(cyParser *parser);

static inline int isCommandPart(cyTT tt) {
    return tt == TT_IDENT || tt == TT_STRING || tt == TT_VARNAME ||
           tt == TT_LPAREN || tt == TT_LBRACKET;
}

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
            advance(parser);
            return root;
        }
        default:
            printf("cysh: invalid command part node type %d\n", tok.type);
            exit(1);
    }
}

static cyNode *parseCommand(cyParser *parser) {
    cyNodeList list = newList(0);

    while (isCommandPart(parser->current.type))
        listPush(&list, parseCommandPart(parser));

    if (parser->current.type == TT_SEMI) advance(parser);

    cyNode *result = newLeaf(NT_COMMAND, NULL, 0);
    result->list = list;
    return result;
}

static cyNode *parseCommandList(cyParser *parser) {
    cyNodeList list = newList(0);

    while (isCommandPart(parser->current.type))
        listPush(&list, parseCommand(parser));

    cyNode *result = newLeaf(NT_COMMANDLIST, NULL, 0);
    result->list = list;
    return result;
}

void cyParserInit(cyParser *parser, cyLex *lex) {
    parser->lex = lex;
    parser->current = cyLexNextToken(lex);
}

cyNode *cyParse(cyParser *parser) { return parseCommandList(parser); }
