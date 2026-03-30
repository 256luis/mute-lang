#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

#define MAX_SYMBOL_LENGTH 256

#define TOK_RESERVED_SYMBOL_COUNT (TOK_RSHIFT + 1)
typedef enum TokenKind
{
    // reserved symbols
    TOK_ROUTINE,
    TOK_FUNC,
    TOK_PROC,
    TOK_ECHO,
    TOK_LET,
    TOK_CONST,
    TOK_STRUCT,
    TOK_ENUM,
    TOK_TYPE,
    TOK_IF,
    TOK_WHILE,
    TOK_MATCH,
    TOK_FOR,

    TOK_ARROW,
    TOK_COMMA,
    TOK_COLON,
    TOK_DOT,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_LBRACE, TOK_RBRACE,
    TOK_LPAREN, TOK_RPAREN,
    TOK_SEMICOLON,
    TOK_EQUAL,
    TOK_AND,
    TOK_PLUS,
    TOK_DASH,
    TOK_SLASH,
    TOK_STAR,
    TOK_PERCENT,
    TOK_DOUBLEEQUAL,
    TOK_BANGEQUAL,
    TOK_LESS,
    TOK_LESSEQUAL,
    TOK_GREAT,
    TOK_GREATEQUAL,
    TOK_DOUBLEAND,
    TOK_DOUBLELINE,
    TOK_BANG,
    TOK_LINE,
    TOK_TILDE,
    TOK_CARET,
    TOK_LSHIFT,
    TOK_RSHIFT,

    // user defined
    TOK_IDENT,
    TOK_NUM,
    TOK_STRING,

} TokenKind;

typedef struct Token
{
    TokenKind kind;
    char string[MAX_SYMBOL_LENGTH];
    int line;
    int column;
} Token;

typedef struct TokenList
{
    Token* tokens;
    size_t count;
    size_t capacity;
} TokenList;

TokenList tokenize(char* source_code);
void print_token(Token t);

#endif
