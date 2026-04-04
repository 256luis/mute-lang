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
    ANK_BINARY,

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

// Enum containing all binary operator types
typedef enum BinaryOperator
{
    // arithmetic
    BO_ADD, BO_SUB, BO_MUL, BO_DIV, BO_MOD,

    // bitwise
    BO_BIT_AND, BO_BIT_OR, BO_BIT_XOR,
    BO_BIT_LSHIFT, BO_BIT_RSHIFT,

    // logical
    BO_LOG_AND, BO_LOG_OR,

    // comparison
    BO_CMP_EQ, BO_CMP_NEQ, BO_CMP_LESS,
    BO_CMP_LESSEQ, BO_CMP_GREAT, BO_CMP_GREATEQ,

    // field access
    BO_ACCESS,
} BinaryOperator;

// The primary node type
typedef struct AstNode
{
    Token token;
    AstNodeKind kind;

    union
    {
        // for ANK_IDENT, ANK_NUM, ANK_STRING
        Token terminal;

        struct
        {
            AstNode* left;
            AstNode* right;
            BinaryOperator op;

            // for error reporting and printing purposes
            Token op_token;
        } binary;
    };
} AstNode;

// cmon u know what this function does
AstNode* parse(TokenList tl);

// prints an AstNode
void print_ast_node(AstNode node);

#endif
