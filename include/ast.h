#ifndef AST_H
#define AST_H

#include "token.h"
#include <stdbool.h>

typedef struct AstNode AstNode;

// Enum containing the different kinds of nodes based on the language's grammar
typedef enum AstNodeKind
{
    // base node kinds
    ANK_ROUTINE_DECLARATION,

    // just rvalue
    ANK_STRING,
    ANK_INT,

    // lvalue/rvalue
    ANK_IDENT,
} AstNodeKind;

/*
  Struct to more easily manage a list of AstNodes.

  Do NOT manually intantiate or mutate members of the struct. See the anl_* functions
  for various operations.
*/
typedef struct AstNodeList
{
    AstNode* nodes;
    size_t count;
    size_t capacity;
} AstNodeList;

// The primary node type
typedef struct AstNode
{
    Token token;
    AstNodeKind kind;

    union
    {
        // for ANK_IDENT, ANK_NUM, ANK_STRING
        Token terminal;
    };
} AstNode;

// cmon u know what this function does
AstNode* parse(TokenList tl);

#endif
