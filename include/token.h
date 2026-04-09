#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

// The maximum length of a tokenizable symbol
#define MAX_SYMBOL_LENGTH 256

// The number of reserved symbols recognized by the tokenizer
#define TOK_RESERVED_SYMBOL_COUNT (TOK_EQUAL + 1)

// Enum containing different token kinds recognized by the tokenizer
typedef enum TokenKind
{
    // reserved keywords
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

    // binary operators
    TOK_DOT,
    TOK_AND,
    TOK_LINE,
    TOK_PLUS,
    TOK_CARET,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_DOUBLEEQUAL,
    TOK_BANGEQUAL,
    TOK_LESS,
    TOK_LESSEQUAL,
    TOK_GREAT,
    TOK_GREATEQUAL,
    TOK_DOUBLEAND,
    TOK_DOUBLELINE,
    TOK_LSHIFT,
    TOK_RSHIFT,
    TOK_STAR,

    // binary/unary operators
    TOK_DASH,

    // unary operators
    TOK_BANG,
    TOK_TILDE,

    // everything else
    TOK_ARROW,
    TOK_COMMA,
    TOK_COLON,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_LBRACE, TOK_RBRACE,
    TOK_LPAREN, TOK_RPAREN,
    TOK_SEMICOLON,
    TOK_EQUAL,

    // user defined
    TOK_IDENT,
    TOK_NUM,
    TOK_STRING,

    TOK_EOF,
} TokenKind;

typedef struct Token
{
    TokenKind kind;
    char string[MAX_SYMBOL_LENGTH];
    int line;
    int column;
} Token;

/*
  Struct to more easily manage a list of tokens.

  Do NOT manually intantiate or mutate members of the struct. See the tl_* functions
  for various operations.
 */
typedef struct TokenList
{
    Token* tokens;
    size_t count;
    size_t capacity;
} TokenList;

// Creates and initializes a TokenList.
TokenList tl_new();

/*
  Appends a token to a token list.

  Parameters:
  - TokenList* tl - a pointer to the TokenList to append to
  - Token token   - the token to append
*/
void tl_append(TokenList* tl, Token token);

/*
  Tokenizes mute source code.

  Parameters:
  - char* source_code - the source code in string form

  Returns a TokenList that contains the tokens scanned from the provided source code.
 */
TokenList tokenize(char* source_code);

// do i really need to write documentation for this
void print_token(Token t);

#endif
