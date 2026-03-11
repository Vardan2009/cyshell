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
    NT_VAR,
    NT_COMMANDLIST,
    NT_COMMAND,
} cyNT;

typedef struct _cyNode {
    cyNT type;

    struct _cyNode *children;
    size_t childCount;

    const char *start;
    size_t len;
} cyNode;

void cyParserInit(cyParser *parser, cyLex *lex);
cyNode *cyParse(cyParser *parser);

#endif  // _CYSH_PARSER
