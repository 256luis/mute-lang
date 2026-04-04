#include <stdbool.h>
#include "token.h"
#include "ast.h"
#include "utils.h"

// Starting capacity of AstNodeList
#define ANL_START_CAPACITY 10

// Growth rate of AstNodeList
#define ANL_GROWTH_RATE 2

// List of binary operator tokens to be used in the EXPECT macro
#define BINARY_OPERATOR_TOKENS\
    TOK_DOT, TOK_AND, TOK_LINE, TOK_PLUS, TOK_CARET, TOK_SLASH, TOK_PERCENT, TOK_DOUBLEEQUAL, TOK_BANGEQUAL, TOK_LESS, TOK_LESSEQUAL, TOK_GREAT, TOK_GREATEQUAL, TOK_DOUBLEAND, TOK_DOUBLELINE, TOK_LSHIFT, TOK_RSHIFT

/*
  Macro for more convenient implementation of language grammar. Reports error and
  exits if expectation is not met.

  Parameters:
  - Parser* p - parser object
  - ...       - list of expected tokens at this point

  Example:

    EXPECT_TOKEN(p, TOK_LET);

    advance(p);
    EXPECT_TOKEN(p, TOK_IDENT);

    advance(p);
    EXPECT_TOKEN(p, TOK_EQUAL);

    // so on...
*/
#define EXPECT_TOKEN(p, ...)\
    do {\
        Token t = current_token(p);\
        bool result = _check_token(p,\
                (TokenKind[]){ __VA_ARGS__ },\
                sizeof((TokenKind[]){ __VA_ARGS__ })/sizeof(TokenKind));\
        if (!result)\
        {\
            print_token(t);\
            ERROR("unexpected symbol `%s` @ line %d\n", t.string, t.line);\
        }\
    } while(0)

/*
  Macro for more convenient implementation of language grammar.

  Parameters:
  - Parser* p - parser object
  - ...       - list of expected tokens at this point

  Returns true if the current token's kind matches one of the token kinds provided.
  False otherwise.

  Example:

    if (CHECK_TOKEN(p, SOME_TOKEN_KIND))
    {
        // ...
    }
*/
#define CHECK_TOKEN(p, ...)\
    _check_token(p,\
                 (TokenKind[]){ __VA_ARGS__ },\
                 sizeof((TokenKind[]){ __VA_ARGS__ })/sizeof(TokenKind))

// Parser struct containing information about the current parsing context
typedef struct Parser
{
    TokenList tl;
    int index;
} Parser;

/*
  Advances the parser i.e. next token please!
*/
static void advance(Parser* p)
{
    p->index++;
}

// what do u think this does...
static Token current_token(Parser* p)
{
    return p->tl.tokens[p->index];
}

// dont use this function!!!!!!!!
static bool _check_token(Parser* p, TokenKind* tks, size_t tks_length)
{
    Token t = current_token(p);
    for (size_t i = 0; i < tks_length; i++)
    {
        if (tks[i] == t.kind)
        {
            return true;
        }
    }

    return false;
}

// Creates and initializes a new AstNodeList
static AstNodeList anl_new()
{
    AstNodeList anl = {
        .nodes = MALLOC(ANL_START_CAPACITY * sizeof(AstNode)),
        .capacity = ANL_START_CAPACITY,
        .count = 0,
    };

    return anl;
}

/*
  Appends an AstNode to an AstNodeList.

  Parameters:
  - AstNodeList* anl - a pointer to the AstNodeList to append to
  - AstNode node     - the node to append
*/
static void anl_append(AstNodeList* anl, AstNode node)
{
    // if not enough capacity
    if ((anl->count + 1) > anl->capacity )
    {
        // expand array
        size_t new_capacity = anl->capacity * ANL_GROWTH_RATE;
        anl->nodes = REALLOC(anl->nodes, new_capacity * sizeof(AstNode));
        anl->capacity = new_capacity;
    }

    anl->nodes[anl->count] = node;
    anl->count++;
}

static AstNode* parse_terminal(Parser* p)
{
    AstNode* rvalue = MALLOC(sizeof(AstNode));
    switch (current_token(p).kind)
    {
        case TOK_IDENT:
        {
            rvalue->kind = ANK_IDENT;
            rvalue->terminal = current_token(p);
        } break;

        case TOK_NUM:
        {
            rvalue->kind = ANK_INT;
            rvalue->terminal = current_token(p);
        } break;

        case TOK_STRING:
        {
            rvalue->kind = ANK_STRING;
            rvalue->terminal = current_token(p);
        } break;

        default:
        {
            UNREACHABLE();
        }
    }

    return rvalue;
}

/*
  Converts a TokenKind into its corresponding BinaryOperator

  Parameters:
  - TokenKind tk - the TokenKind to turn into a BinaryOperator

  Returns a BinaryOperator
*/
static BinaryOperator token_kind_to_binary_operator(TokenKind tk)
{
    static BinaryOperator map[] = {
        [TOK_DOT] = BO_ACCESS,
        [TOK_AND] = BO_BIT_AND,
        [TOK_LINE] = BO_BIT_OR,
        [TOK_PLUS] = BO_ADD,
        [TOK_CARET] = BO_BIT_XOR,
        [TOK_SLASH] = BO_DIV,
        [TOK_PERCENT] = BO_MOD,
        [TOK_DOUBLEEQUAL] = BO_CMP_EQ,
        [TOK_BANGEQUAL] = BO_CMP_NEQ,
        [TOK_LESS] = BO_CMP_LESS,
        [TOK_LESSEQUAL] = BO_CMP_LESSEQ,
        [TOK_GREAT] = BO_CMP_GREAT,
        [TOK_GREATEQUAL] = BO_CMP_GREATEQ,
        [TOK_DOUBLEAND] = BO_LOG_AND,
        [TOK_DOUBLELINE] = BO_LOG_OR,
        [TOK_LSHIFT] = BO_BIT_LSHIFT,
        [TOK_RSHIFT] = BO_BIT_RSHIFT,
        [TOK_DASH] = BO_SUB,
        [TOK_STAR] = BO_MUL,
    };

    // TOK_DOT and TOK_STAR are the starting and ending binary operator token kinds
    ASSERT(tk >= TOK_DOT && tk <= TOK_STAR);

    return map[tk];
}

/*
  Parses an rvalue. Rvalues are values that belong on the right side of an assignment
  operation.

  Parameters:
  - Parser* p - parser

  Returns an AstNode that is your rvalue.
*/
AstNode* parse_rvalue(Parser* p)
{
    AstNode* rvalue = NULL;
    switch (current_token(p).kind)
    {
        case TOK_IDENT:
        case TOK_NUM:
        case TOK_STRING:
        {
            rvalue = parse_terminal(p);
        } break;

        default:
        {
            UNIMPLEMENTED();
        }
    }

    ASSERT(rvalue != NULL);

    // TODO: operator precedence

    advance(p);
    while (CHECK_TOKEN(p, BINARY_OPERATOR_TOKENS))
    {
        AstNode* left = rvalue;

        rvalue = MALLOC(sizeof(AstNode));
        rvalue->kind = ANK_BINARY;
        rvalue->binary.op = token_kind_to_binary_operator(current_token(p).kind);
        rvalue->binary.op_token = current_token(p);

        advance(p);
        EXPECT_TOKEN(p, TOK_IDENT, TOK_NUM, TOK_STRING);
        AstNode* right = parse_terminal(p);

        rvalue->binary.right = right;
        rvalue->binary.left = left;

        advance(p);
    }

    ASSERT(rvalue != NULL);

    return rvalue;
}

/*
  Parses a statement.

  Parameters:
  - Parser* p - parser

  Returns an AstNode that is your statement.
*/
static AstNode* parse_statement(Parser* p)
{
    AstNode* node = NULL;

    switch (current_token(p).kind)
    {
        default:
        {
            UNIMPLEMENTED();
        }
    }

    ASSERT(node != NULL);

    return node;
}

AstNode* parse(TokenList tl)
{
    Parser p = {
        .tl = tl,
        .index = 0,
    };

    return parse_rvalue(&p);
}

void print_ast_node(AstNode node)
{
#define NEWLINE()\
    do {\
        putchar('\n');\
        for (int i = 0; i < depth; i++)\
        {\
            printf("    ");\
        }\
    } while (0)

    static int depth = 0;

    switch (node.kind)
    {
        case ANK_ROUTINE_DECLARATION:
        {
            UNIMPLEMENTED();
        } break;

        case ANK_STRING:
        {
            printf("STRING: %s", node.terminal.string);
        } break;

        case ANK_INT:
        {
            printf("INT: %s", node.terminal.string);
        } break;

        case ANK_IDENT:
        {
            printf("IDENT: %s", node.terminal.string);
        } break;

        case ANK_BINARY:
        {
            printf("BINARY {");
            depth++;
            NEWLINE();

            printf("op: %s", node.binary.op_token.string);

            NEWLINE();
            printf("left: ");
            print_ast_node(*node.binary.left);

            NEWLINE();
            printf("right: ");
            print_ast_node(*node.binary.right);


            depth--;
            NEWLINE();
            printf("}");
        } break;

        default:
        {
            UNIMPLEMENTED();
        }
    }
#undef NEWLINE
}
