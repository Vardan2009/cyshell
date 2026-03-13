#ifndef _CYSH_PARSER
#define _CYSH_PARSER

#include "lex.h"

typedef struct {
    cyLex *lex;
    cyTok current;
} cyParser;

typedef enum {
    NT_IDENT,
    NT_STRING,
    NT_NUMBER,
    NT_VAR,
    NT_COMMANDLIST,
    NT_COMMAND,
    NT_BINOP,
} cyNT;

typedef struct _cyNode cyNode;

typedef struct {
    cyNode **data;
    size_t count;
    size_t capacity;
} cyNodeList;

typedef struct _cyNode {
    cyNT type;
    cyNodeList list;
    union {
        struct {
            const char *start;
            size_t len;
        };
        cyTT tt;
    };
} cyNode;

void cyParserInit(cyParser *parser, cyLex *lex);
cyNode *cyParse(cyParser *parser);

void cyNodePrint(cyNode *node, int indent);
void cyNodeFree(cyNode *node);

#endif  // _CYSH_PARSER
