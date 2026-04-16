#ifndef AST_H
#define AST_H

#include "token.h"
#include <stdbool.h>

typedef struct AstNode AstNode;

// Enum containing the different kinds of nodes based on the language's grammar
typedef enum AstNodeKind
{
    // base node kinds
    ANK_VARIABLE_DECL,
    ANK_ROUTINE_DECL,

    // base/rvalue
    ANK_ROUTINE_CALL,

    // just rvalue
    ANK_STRING,
    ANK_INT,
    ANK_FLOAT,
    ANK_BINARY,
    ANK_UNARY,
    ANK_ARRAY_INDEX,
    ANK_ARRAY_TYPE,
    ANK_ARRAY_INIT,
    ANK_STRUCT_INIT,

    // lvalue/rvalue
    ANK_IDENT,

    // sometimes valid lvalue, sometimes not
    ANK_FIELD_ACCESS,
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
} BinaryOperator;

typedef enum UnaryOperator
{
    UO_NEG,

    UO_LOG_NOT,
    UO_BIT_NOT,
} UnaryOperator;

// The primary node type
typedef struct AstNode
{
    Token token;
    AstNodeKind kind;

    union
    {
        // for ANK_IDENT, ANK_INT, ANK_STRING
        Token terminal;

        struct
        {
            Token whole;
            Token fractional;
        } floating;

        struct
        {
            AstNode* left;
            AstNode* right;
            BinaryOperator op;

            // for error reporting and printing purposes
            Token op_token;
        } binary;

        struct
        {
            UnaryOperator op;
            AstNode* node;

            // for error reporting and printing purposes
            Token op_token;
        } unary;

        struct
        {
            AstNode* owner;
            AstNode* field;
        } field_access;

        struct
        {
            AstNode* routine;
            AstNodeList args;
        } routine_call;

        struct
        {
            AstNode* array;
            AstNode* index;
        } array_index;

        struct
        {
            // how big is the array?
            AstNode* size; // nullable

            // array of what?
            AstNode* child_type_node;
        } array_type_node;

        struct
        {
            AstNode* type_node; // nullable
            AstNodeList elems;
        } array_init;

        struct
        {
            AstNode* type_node; // nullable
            TokenList member_idents;
            AstNodeList member_inits;
        } struct_init;


        struct
        {
            Token ident;
            bool is_mutable;
            AstNode* type_node; // nullable
            AstNode* rvalue;
        } variable_decl;
    };
} AstNode;

// cmon u know what this function does
AstNode* parse(TokenList tl);

// prints an AstNode
void print_ast_node(AstNode node);

#endif
