#ifndef _CYSH_PARSER
#define _CYSH_PARSER

#include "lex.h"

typedef enum {
    NT_COMMAND,
} cyNT;

typedef struct {
    cyNT type;
} cyNode;

#endif  // _CYSH_PARSER
