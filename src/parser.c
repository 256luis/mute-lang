#include <stdbool.h>
#include "token.h"
#include "ast.h"
#include "utils.h"

#define ANL_START_CAPACITY 10
#define ANL_GROWTH_RATE 2

/*
  Macro for more convenient implementation of language grammar. Reports error and
  exits if expectation is not met.

  Parameters:
  - Parser* p - parser object
  - ...       - list of expected tokens at this point

  Example:

    EXPECT(p, TOK_LET);

    advance(p);
    EXPECT(p, TOK_IDENT);

    advance(p);
    EXPECT(p, TOK_EQUAL);

    // so on...
*/
#define EXPECT(p, ...)\
    do {\
        Token t = current_token(p);\
        bool result = _expect(p,\
                (TokenKind[]){ __VA_ARGS__ },\
                sizeof((TokenKind[]){ __VA_ARGS__ })/sizeof(TokenKind));\
        if (!result)\
        {\
            ERROR("unexpected symbol `%s` @ line %d\n", t.string, t.line);\
        }\
    } while(0)

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

// dont use this function!!!!!!!!
static bool _expect(Parser* p, TokenKind* tks, size_t tks_length)
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

/*
  Parses an rvalue. Rvalues are values that belong on the right side of an assignment
  operation.

  Parameters:
  - Parser* p - parser

  Returns an AstNode that is your rvalue.
*/
static AstNode* parse_rvalue(Parser* p)
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
            UNIMPLEMENTED();
        }
    }

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
    AstNode* node;

    switch (current_token(p).kind)
    {
        default:
        {
            UNIMPLEMENTED();
        }
    }

    return node;
}

AstNode* parse(TokenList tl)
{
    Parser p = {
        .tl = tl,
        .index = 0,
    };

    return parse_statement(&p);
}
