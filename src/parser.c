#include <stdbool.h>
#include <string.h>
#include "token.h"
#include "ast.h"
#include "utils.h"

// Starting capacity of AstNodeList
#define ANL_START_CAPACITY 10

// Growth rate of AstNodeList
#define ANL_GROWTH_RATE 2

// List of binary operator tokens to be used in the EXPECT macro
#define BINARY_OPERATOR_TOKENS\
    TOK_AND, TOK_LINE, TOK_PLUS, TOK_CARET, TOK_SLASH, TOK_PERCENT, TOK_DOUBLEEQUAL, TOK_BANGEQUAL, TOK_LESS, TOK_LESSEQUAL, TOK_GREAT, TOK_GREATEQUAL, TOK_DOUBLEAND, TOK_DOUBLELINE, TOK_LSHIFT, TOK_RSHIFT, TOK_DASH

// List of TokenKinds that could be the beginning of an rvalue expression
#define RVALUE_STARTERS\
    TOK_LBRACE, TOK_IF, TOK_DOT, TOK_IDENT, TOK_NUM, TOK_STRING, TOK_LPAREN, TOK_DASH, TOK_BANG, TOK_TILDE, TOK_LBRACKET

#define LVALUE_STARTERS\
    TOK_IDENT

#define TYPE_STARTERS\
    TOK_IDENT, TOK_LBRACKET

#define STATEMENT_STARTERS\
    TOK_RETURN, TOK_CONST, TOK_IDENT, TOK_LET, TOK_IF, TOK_LBRACE

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
        bool result = _check_nth_token(p, 0,\
                (TokenKind[]){ __VA_ARGS__ },\
                sizeof((TokenKind[]){ __VA_ARGS__ })/sizeof(TokenKind));\
        if (!result)\
        {\
            DEBUG(printf("called from line %d\n", __LINE__));\
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
    _check_nth_token(p, 0,\
                 (TokenKind[]){ __VA_ARGS__ },\
                 sizeof((TokenKind[]){ __VA_ARGS__ })/sizeof(TokenKind))

#define CHECK_NTH_TOKEN(p, n, ...)\
    _check_nth_token(p, n,\
                 (TokenKind[]){ __VA_ARGS__ },\
                 sizeof((TokenKind[]){ __VA_ARGS__ })/sizeof(TokenKind))

/*
  Checks if a string of tokens starting from the current token matches the provided
  pattern of tokens.

  Parameters:
  - Parser* p - parser object
  - ...       - list of TokenKinds to match

  Returns true if the pattern matches. False otherwise.
*/
#define CHECK_TOKEN_PATTERN(p, ...)\
    _check_token_pattern(p,\
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

/*
  Returns the nth token starting from the current token

  Parameters:
  - Parser* p - parser
  - int n     - the offset from the current token
*/
static Token nth_token(Parser* p, int n)
{
    return p->tl.tokens[p->index + n];
}

// dont use this function!!! use the CHECK_TOKEN_PATTERN macro
static bool _check_token_pattern(Parser* p, TokenKind* tks, size_t tks_length)
{
    bool matches = true;
    for (size_t i = 0; i < tks_length; i++)
    {
        if (nth_token(p, i).kind != tks[i])
        {
            matches = false;
        }
    }

    return matches;
}

// dont use this function!!!!!!!!
static bool _check_nth_token(Parser* p, int n, TokenKind* tks, size_t tks_length)
{
    Token t = nth_token(p, n);
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

// forward declaration
static AstNode* parse_rvalue(Parser* parser);
static AstNode* parse_term(Parser* parser);
static AstNode* parse_stmt(Parser* p);

/*
  Converts a TokenKind into its corresponding UnaryOperator

  Parameters:
  - TokenKind tk - the TokenKind to turn into a UnaryOperator

  Returns a UnaryOperator
*/
static UnaryOperator token_kind_to_unary_operator(TokenKind tk)
{
    static UnaryOperator map[] = {
        [TOK_DASH] = UO_NEG,
        [TOK_BANG] = UO_LOG_NOT,
        [TOK_TILDE] = UO_BIT_NOT,
    };

    // TOK_DASH and TOK_TILDE are the starting and ending unary operator token kinds
    // if this assertion fails, tk is not a unary token kind
    ASSERT(tk >= TOK_DASH && tk <= TOK_TILDE);

    return map[tk];
}

static AstNode* parse_unary(Parser* p)
{
    AstNode* rvalue = MALLOC(sizeof(AstNode));
    rvalue->kind = ANK_UNARY;
    rvalue->unary.op_token = current_token(p);
    rvalue->unary.op = token_kind_to_unary_operator(current_token(p).kind);

    advance(p);
    rvalue->unary.node = parse_term(p);

    return rvalue;
}

static AstNode* parse_lvalue(Parser* p)
{
    CHECK_TOKEN(p, LVALUE_STARTERS);
    return parse_term(p);
}

static AstNode* parse_array_init(Parser* p, AstNode* type_node)
{
    AstNode* node = MALLOC(sizeof(AstNode));
    node->kind = ANK_ARRAY_INIT;
    node->array_init.type_node = type_node;
    node->array_init.elems = anl_new();

    // not checking these two tokens because we already know that this is an array
    // initializer
    advance(p); // .
    advance(p); // [

    // [n]T.[x]
    //       ^--- we are here

    // parse the intialized elements
    while (!CHECK_TOKEN(p, TOK_RBRACKET))
    {
        AstNode* elem = parse_rvalue(p);
        anl_append(&node->array_init.elems, *elem);

        advance(p);
        EXPECT_TOKEN(p, TOK_COMMA, TOK_RBRACKET);
        if (CHECK_TOKEN(p, TOK_COMMA))
        {
            advance(p);
            // EXPECT_TOKEN(p, RVALUE_STARTERS);
        }
    }

    return node;
}

static AstNode* parse_struct_init(Parser* p, AstNode* type_node)
{
    AstNode* node = MALLOC(sizeof(AstNode));
    node->kind = ANK_STRUCT_INIT;
    node->struct_init.type_node = type_node;
    node->struct_init.member_idents = tl_new();
    node->struct_init.member_inits = anl_new();

    // not checking these two tokens because we already know that this is a struct
    // initializer
    advance(p); // .
    advance(p); // {

    // T.{ x = y }
    //     ^--- we are here

    while (!CHECK_TOKEN(p, TOK_RBRACE))
    {
        EXPECT_TOKEN(p, TOK_IDENT);
        tl_append(&node->struct_init.member_idents, current_token(p));

        advance(p);
        EXPECT_TOKEN(p, TOK_EQUAL);

        advance(p);
        AstNode* member_init = parse_rvalue(p);
        anl_append(&node->struct_init.member_inits, *member_init);

        advance(p);
        EXPECT_TOKEN(p, TOK_COMMA, TOK_RBRACE);
        if (CHECK_TOKEN(p, TOK_COMMA))
        {
            advance(p);
            // EXPECT_TOKEN(p, TOK_IDENT);
        }
    }

    return node;
}

static AstNode* parse_type(Parser* p)
{
    EXPECT_TOKEN(p, TYPE_STARTERS);

    AstNode* type_node = NULL;

    switch (current_token(p).kind)
    {
        case TOK_IDENT:
        {
            // identical to what's in parse_term
            type_node = MALLOC(sizeof(AstNode));
            type_node->kind = ANK_IDENT;
            type_node->terminal = current_token(p);
        } break;

        // array type
        case TOK_LBRACKET:
        {
            type_node = MALLOC(sizeof(AstNode));
            type_node->kind = ANK_ARRAY_TYPE;
            type_node->array_type_node.size = NULL;

            advance(p);

            // size of array is optional
            if (!CHECK_TOKEN(p, TOK_RBRACKET))
            {
                type_node->array_type_node.size = parse_rvalue(p);
                advance(p);
            }

            EXPECT_TOKEN(p, TOK_RBRACKET);

            advance(p);
            type_node->array_type_node.child_type_node = parse_type(p);
        } break;

        default:
        {
            UNREACHABLE();
        }
    }

    return type_node;
}

static AstNode* parse_if(Parser* p)
{
    AstNode* node = MALLOC(sizeof(AstNode));
    node->kind = ANK_IF;
    node->if_stmt.on_false = NULL;

    advance(p);
    node->if_stmt.cond = parse_rvalue(p);

    advance(p);
    node->if_stmt.on_true = parse_stmt(p);

    if (CHECK_NTH_TOKEN(p, 1, TOK_ELSE))
    {
        advance(p);
        advance(p);
        node->if_stmt.on_false = parse_stmt(p);
    }

    return node;
}

static AstNode* parse_compound(Parser* p)
{
    AstNode* node = MALLOC(sizeof(AstNode));
    node->kind = ANK_COMPOUND;
    node->compound.stmts = anl_new();

    advance(p);
    while (!CHECK_TOKEN(p, TOK_RBRACE))
    {
        AstNode* stmt = parse_stmt(p);
        anl_append(&node->compound.stmts, *stmt);
        advance(p);
    }

    return node;
}

static AstNode* parse_routine_call(Parser* p, AstNode* routine)
{
    AstNode* node = MALLOC(sizeof(AstNode));
    node->kind = ANK_ROUTINE_CALL;
    node->routine_call.routine = routine;
    node->routine_call.args = anl_new();

    advance(p);

    // parse the args
    while (!CHECK_TOKEN(p, TOK_RPAREN))
    {
        AstNode* arg = parse_rvalue(p);
        anl_append(&node->routine_call.args, *arg);

        advance(p);
        EXPECT_TOKEN(p, TOK_COMMA, TOK_RPAREN);

        if (CHECK_TOKEN(p, TOK_COMMA))
        {
            advance(p);
            EXPECT_TOKEN(p, RVALUE_STARTERS);
        }
    }

    return node;
}

/*
  Parses a single term in an expression. Expressions enclosed in parentheses count
  as a single term.

  Parameters:
  - Parser* p - parser

  Returns an AstNode that is your expression.
*/
static AstNode* parse_term(Parser* p)
{
    EXPECT_TOKEN(p, RVALUE_STARTERS);

    AstNode* rvalue = NULL;

    // array initializer with inferred type
    if (CHECK_TOKEN_PATTERN(p, TOK_DOT, TOK_LBRACKET))
    {
        rvalue = parse_array_init(p, NULL);
    }
    // struct initializer with inferred type
    else if (CHECK_TOKEN_PATTERN(p, TOK_DOT, TOK_LBRACE))
    {
        rvalue = parse_struct_init(p, NULL);
    }
    else
    {
        switch (current_token(p).kind)
        {
            case TOK_IDENT:
            {
                rvalue = MALLOC(sizeof(AstNode));
                rvalue->kind = ANK_IDENT;
                rvalue->terminal = current_token(p);
            } break;

            case TOK_NUM:
            {
                rvalue = MALLOC(sizeof(AstNode));

                // check if floating point
                if(CHECK_TOKEN_PATTERN(p, TOK_NUM, TOK_DOT, TOK_NUM))
                {
                    rvalue->kind = ANK_FLOAT;
                    rvalue->floating.whole = current_token(p);

                    advance(p);
                    advance(p);
                    rvalue->floating.fractional = current_token(p);
                }
                // must be int
                else
                {
                    rvalue->kind = ANK_INT;
                    rvalue->terminal = current_token(p);
                }
            } break;

            case TOK_STRING:
            {
                rvalue = MALLOC(sizeof(AstNode));
                rvalue->kind = ANK_STRING;
                rvalue->terminal = current_token(p);
            } break;

            case TOK_LPAREN:
            {
                advance(p);
                rvalue = parse_rvalue(p);

                advance(p);
                EXPECT_TOKEN(p, TOK_RPAREN);
            } break;

            case TOK_DASH:
            case TOK_BANG:
            case TOK_TILDE:
            {
                rvalue = parse_unary(p);
            } break;

            // array type
            case TOK_LBRACKET:
            {
                rvalue = parse_type(p);
            } break;

            case TOK_IF:
            {
                rvalue = parse_if(p);
            } break;

            case TOK_LBRACE:
            {
                rvalue = parse_compound(p);
            } break;

            default:
            {
                UNIMPLEMENTED();
            }
        }
    }

    while (CHECK_NTH_TOKEN(p, 1, TOK_DOT, TOK_LPAREN, TOK_LBRACKET))
    {
        advance(p);

        // array initializer with explicit type
        if (CHECK_TOKEN_PATTERN(p, TOK_DOT, TOK_LBRACKET))
        {
            rvalue = parse_array_init(p, rvalue);
        }
        else if (CHECK_TOKEN_PATTERN(p, TOK_DOT, TOK_LBRACE))
        {
            rvalue = parse_struct_init(p, rvalue);
        }
        else
        {
            switch (current_token(p).kind)
            {
                // field access
                case TOK_DOT:
                {
                    AstNode* owner = rvalue;
                    rvalue = MALLOC(sizeof(AstNode));
                    rvalue->kind = ANK_FIELD_ACCESS;
                    rvalue->field_access.owner = owner;

                    advance(p);

                    rvalue->field_access.field = parse_lvalue(p);
                } break;

                // routine call
                case TOK_LPAREN:
                {
                    rvalue = parse_routine_call(p, rvalue);
                } break;

                // array index
                case TOK_LBRACKET:
                {
                    AstNode* array = rvalue;
                    rvalue = MALLOC(sizeof(AstNode));
                    rvalue->kind = ANK_ARRAY_INDEX;
                    rvalue->array_index.array = array;

                    advance(p);
                    rvalue->array_index.index = parse_rvalue(p);
                    advance(p);
                    EXPECT_TOKEN(p, TOK_RBRACKET);
                } break;

                default:
                {
                    UNIMPLEMENTED();
                }
            }
        }
    }

    ASSERT(rvalue != NULL);

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
        [TOK_STAR] = BO_MUL,
        [TOK_DASH] = BO_SUB,
    };

    // TOK_AND and TOK_DASH are the starting and ending binary operator token kinds
    // if this assertion fails, tk is not a binary token kind
    ASSERT(tk >= TOK_AND && tk <= TOK_DASH);

    return map[tk];
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
    AstNode* rvalue = parse_term(p);

    // TODO: operator precedence

    // advance(p);
    while (CHECK_NTH_TOKEN(p, 1, BINARY_OPERATOR_TOKENS))
    {
        advance(p);

        AstNode* left = rvalue;

        rvalue = MALLOC(sizeof(AstNode));
        rvalue->kind = ANK_BINARY;
        rvalue->binary.op = token_kind_to_binary_operator(current_token(p).kind);
        rvalue->binary.op_token = current_token(p);

        advance(p);
        AstNode* right = parse_term(p);

        rvalue->binary.right = right;
        rvalue->binary.left = left;

        // advance(p);
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
static AstNode* parse_stmt(Parser* p)
{
    EXPECT_TOKEN(p, STATEMENT_STARTERS, RVALUE_STARTERS);

    AstNode* node = NULL;

    // variable reassignment
    if (CHECK_TOKEN_PATTERN(p, TOK_IDENT, TOK_EQUAL))
    {
        node = MALLOC(sizeof(AstNode));
        node->kind = ANK_VARIABLE_ASSIGN;

        node->variable_assign.lvalue = parse_lvalue(p);

        advance(p);
        EXPECT_TOKEN(p, TOK_EQUAL);

        advance(p);
        node->variable_assign.rvalue = parse_rvalue(p);

        advance(p);
        EXPECT_TOKEN(p, TOK_SEMICOLON);
    }
    else
    {
        switch (current_token(p).kind)
        {
            // variable & const  declaration
            case TOK_CONST:
            case TOK_LET:
            {
                node = MALLOC(sizeof(AstNode));

                if (CHECK_TOKEN(p, TOK_LET))
                    node->kind = ANK_VARIABLE_DECL;
                else
                    node->kind = ANK_CONST_DECL;

                node->varconst_decl.is_mutable = false;
                node->varconst_decl.type_node = NULL;

                advance(p);
                EXPECT_TOKEN(p, TOK_MUT, TOK_IDENT);

                if (CHECK_TOKEN(p, TOK_MUT))
                {
                    node->varconst_decl.is_mutable = true;
                    advance(p);
                    EXPECT_TOKEN(p, TOK_IDENT);
                }

                node->varconst_decl.ident = current_token(p);

                advance(p);
                EXPECT_TOKEN(p, TOK_EQUAL, TOK_COLON);

                if (CHECK_TOKEN(p, TOK_COLON))
                {
                    advance(p);
                    node->varconst_decl.type_node = parse_term(p);

                    advance(p);
                    EXPECT_TOKEN(p, TOK_EQUAL);
                }

                advance(p);
                node->varconst_decl.rvalue = parse_rvalue(p);

                advance(p);
                EXPECT_TOKEN(p, TOK_SEMICOLON);
            } break;

            case TOK_IF:
            {
                node = parse_if(p);
            } break;

            case TOK_LBRACE:
            {
                node = parse_compound(p);
            } break;

            case TOK_RETURN:
            {
                node = MALLOC(sizeof(AstNode));
                node->kind = ANK_RETURN;

                advance(p);
                node->return_stmt.rvalue = parse_rvalue(p);

                advance(p);
                EXPECT_TOKEN(p, TOK_SEMICOLON);
            } break;

            default:
            {
                if (CHECK_TOKEN(p, RVALUE_STARTERS))
                {
                    node = parse_rvalue(p);
                    if (!CHECK_NTH_TOKEN(p, 1, TOK_SEMICOLON))
                    {
                        // TODO: implicit return
                    }
                }
                else
                {
                    print_token(current_token(p));
                    UNIMPLEMENTED();
                }
            }
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

    return parse_stmt(&p);
}

void print_ast_node(AstNode node)
{
#define NEWLINE()\
    do {\
        putchar('\n');\
        for (int _i = 0; _i < depth; _i++)\
        {\
            printf("    ");\
        }\
    } while (0)

    static int depth = 0;

    switch (node.kind)
    {
        case ANK_ROUTINE_DECL:
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

        case ANK_FLOAT:
        {
            printf("FLOAT: %s.%s", node.floating.whole.string, node.floating.fractional.string);
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

        case ANK_UNARY:
        {
            printf("UNARY {");
            depth++;
            NEWLINE();

            printf("op: %s", node.unary.op_token.string);

            NEWLINE();
            printf("node: ");
            print_ast_node(*node.unary.node);

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_FIELD_ACCESS:
        {
            printf("FIELD ACCESS {");
            depth++;
            NEWLINE();
            printf("owner: ");
            print_ast_node(*node.field_access.owner);

            NEWLINE();
            printf("field: ");
            print_ast_node(*node.field_access.field);

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_ROUTINE_CALL:
        {
            printf("ROUTINE CALL {");
            depth++;
            NEWLINE();

            printf("routine: ");
            print_ast_node(*node.routine_call.routine);

            NEWLINE();
            printf("args: [");
            depth++;
            for (size_t i = 0; i < node.routine_call.args.count; i++)
            {
                NEWLINE();
                AstNode arg = node.routine_call.args.nodes[i];
                print_ast_node(arg);
            }
            depth--;
            if (node.routine_call.args.count > 0) NEWLINE();
            printf("]");

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_ARRAY_INDEX:
        {
            printf("ARRAY INDEX {");
            depth++;
            NEWLINE();

            printf("array: ");
            print_ast_node(*node.array_index.array);

            NEWLINE();
            printf("index: ");
            print_ast_node(*node.array_index.index);

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_ARRAY_TYPE:
        {
            printf("ARRAY TYPE {");
            depth++;
            NEWLINE();

            if (node.array_type_node.size != NULL)
            {
                printf("size: ");
                print_ast_node(*node.array_type_node.size);
                NEWLINE();
            }

            printf("child type: ");
            print_ast_node(*node.array_type_node.child_type_node);

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_ARRAY_INIT:
        {
            printf("ARRAY INIT {");
            depth++;
            NEWLINE();

            if (node.array_init.type_node != NULL)
            {
                printf("type: ");
                print_ast_node(*node.array_init.type_node);
                NEWLINE();
            }

            printf("elems: [");
            depth++;
            for (size_t i = 0; i < node.array_init.elems.count; i++)
            {
                NEWLINE();
                AstNode elem = node.array_init.elems.nodes[i];
                print_ast_node(elem);
            }
            depth--;
            if (node.array_init.elems.count > 0) NEWLINE();
            printf("]");

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_STRUCT_INIT:
        {
            printf("STRUCT INIT {");
            depth++;
            NEWLINE();

            if (node.struct_init.type_node != NULL)
            {
                printf("type: ");
                print_ast_node(*node.struct_init.type_node);
                NEWLINE();
            }

            printf("members: [");
            depth++;
            for (size_t i = 0; i < node.struct_init.member_inits.count; i++)
            {
                NEWLINE();
                printf("%s = ", node.struct_init.member_idents.tokens[i].string);
                print_ast_node(node.struct_init.member_inits.nodes[i]);
            }
            depth--;
            if (node.struct_init.member_inits.count > 0) NEWLINE();
            printf("}");

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_CONST_DECL:
        case ANK_VARIABLE_DECL:
        {
            if (node.kind == ANK_VARIABLE_DECL)
                printf("VARIABLE DECL {");
            else
                printf("CONST DECL {");

            depth++;
            NEWLINE();

            printf("mutable: %s", node.varconst_decl.is_mutable ? "true" : "false");

            NEWLINE();
            printf("ident: %s", node.varconst_decl.ident.string);

            if (node.varconst_decl.type_node != NULL)
            {
                NEWLINE();
                printf("type: ");
                print_ast_node(*node.varconst_decl.type_node);
            }

            NEWLINE();
            printf("value: ");
            print_ast_node(*node.varconst_decl.rvalue);

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_IF:
        {
            printf("IF {");
            depth++;
            NEWLINE();

            printf("cond: ");
            print_ast_node(*node.if_stmt.cond);

            NEWLINE();
            printf("on true: ");
            print_ast_node(*node.if_stmt.on_true);

            if (node.if_stmt.on_false != NULL)
            {
                printf("else: ");
                print_ast_node(*node.if_stmt.on_false);
            }

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_COMPOUND:
        {
            printf("COMPOUND: {");
            depth++;

            for (size_t i = 0; i < node.compound.stmts.count; i++)
            {
                NEWLINE();
                AstNode stmt = node.compound.stmts.nodes[i];
                print_ast_node(stmt);
            }

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_VARIABLE_ASSIGN:
        {
            printf("VARIABLE ASSIGN: {");
            depth++;
            NEWLINE();

            printf("lvalue: ");
            print_ast_node(*node.variable_assign.lvalue);
            NEWLINE();

            printf("rvalue: ");
            print_ast_node(*node.variable_assign.rvalue);

            depth--;
            NEWLINE();
            printf("}");
        } break;

        case ANK_RETURN:
        {
            printf("RETURN: {");
            depth++;
            NEWLINE();

            printf("rvalue: ");
            print_ast_node(*node.return_stmt.rvalue);

            depth--;
            NEWLINE();
            printf("}");
        } break;
    }
#undef NEWLINE
}
