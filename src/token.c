#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "utils.h"

// Starting capacity of TokenList
#define TL_START_CAPACITY 10

// TokenList growth rate
#define TL_GROWTH_RATE 2

// Enum containing the different states of the tokenizer
typedef enum State
{
    STATE_START,
    STATE_WORD,
    STATE_NUMBER,
    STATE_SPECIAL,
    STATE_STRING,
} State;

TokenList tl_new()
{
    TokenList token_list = {
        .tokens = MALLOC(TL_START_CAPACITY * sizeof(Token)),
        .capacity = TL_START_CAPACITY,
        .count = 0,
    };

    return token_list;
}

void tl_append(TokenList* tl, Token token)
{
    // if not enough capacity
    if ((tl->count + 1) > tl->capacity )
    {
        // expand array
        size_t new_capacity = tl->capacity * TL_GROWTH_RATE;
        tl->tokens = REALLOC(tl->tokens, new_capacity * sizeof(Token));
        tl->capacity = new_capacity;
    }

    tl->tokens[tl->count] = token;
    tl->count++;
}

#define TOK_SPECIAL_SYMBOLS_START TOK_DOT
static char* reserved_symbols[] = {
    [TOK_ROUTINE]     = "routine",
    [TOK_FUNC]        = "func",
    [TOK_PROC]        = "proc",
    [TOK_ECHO]        = "echo",
    [TOK_LET]         = "let",
    [TOK_CONST]       = "const",
    [TOK_STRUCT]      = "struct",
    [TOK_ENUM]        = "enum",
    [TOK_TYPE]        = "type",
    [TOK_IF]          = "if",
    [TOK_WHILE]       = "while",
    [TOK_MATCH]       = "match",
    [TOK_FOR]         = "for",

    [TOK_DOT]         = ".",
    [TOK_AND]         = "&",
    [TOK_LINE]        = "|",
    [TOK_PLUS]        = "+",
    [TOK_CARET]       = "^",
    [TOK_SLASH]       = "/",
    [TOK_PERCENT]     = "%",
    [TOK_DOUBLEEQUAL] = "==",
    [TOK_BANGEQUAL]   = "!=",
    [TOK_LESS]        = "<",
    [TOK_LESSEQUAL]   = "<=",
    [TOK_GREAT]       = ">",
    [TOK_GREATEQUAL]  = ">=",
    [TOK_DOUBLEAND]   = "&&",
    [TOK_DOUBLELINE]  = "||",
    [TOK_LSHIFT]      = "<<",
    [TOK_RSHIFT]      = ">>",

    [TOK_DASH]        = "-",
    [TOK_STAR]        = "*",

    [TOK_BANG]        = "!",
    [TOK_TILDE]       = "~",

    [TOK_ARROW]       = "->",
    [TOK_COMMA]       = ",",
    [TOK_COLON]       = ":",
    [TOK_LBRACKET]    = "[",
    [TOK_RBRACKET]    = "]",
    [TOK_LBRACE]      = "{",
    [TOK_RBRACE]      = "}",
    [TOK_LPAREN]      = "(",
    [TOK_RPAREN]      = ")",
    [TOK_SEMICOLON]   = ";",
    [TOK_EQUAL]       = "=",
};

/*
  Turns a string into a Token. Use this function AFTER grouping characters together.

  Parameters:
  - char* symbol - the string to tokenize
  - int line     - line where the string was found
  - int column   - column where the string was found
*/
static Token make_token(char* symbol, int line, int column)
{
    Token token = {
        .line = line,
        .column = column
    };
    strcpy(token.string, symbol);

    // check if reserved symbol
    for (int i = 0; i < TOK_RESERVED_SYMBOL_COUNT; i++)
    {
        if(strcmp(token.string, reserved_symbols[i]) == 0)
        {
            token.kind = i;
            return token;
        }
    }

    // check if number
    if (isdigit(token.string[0]))
    {
        token.kind = TOK_NUM;
    }
    // check if identifier
    else if (token.string[0] == '_' || isalpha(token.string[0]))
    {
        token.kind = TOK_IDENT;
    }
    else
    {
        ERROR("invalid symbol `%s`\n", token.string);
    }

    return token;
}

// this function is very bad and ugly
TokenList tokenize(char* source_code)
{
    TokenList token_list = tl_new();
    char symbol_buffer[MAX_SYMBOL_LENGTH] = {0};
    size_t symbol_buffer_length = 0;

    State state = STATE_START;

    // iterate over source_code
    int source_code_length = strlen(source_code);
    int line = 1;
    int column = 1;
    bool in_comment = false;
    for (int i = 0; i < source_code_length; i++)
    {
        char c = source_code[i];

        column++;
        if (c == '\n')
        {
            line++;
            column = 1;
            in_comment = false;
        }
        else if (c == '#')
        {
            in_comment = true;
        }

        if (in_comment)
        {
            continue;
        }

        if (isspace(c) && symbol_buffer_length == 0)
        {
            continue;
        }

        bool do_make_token = false;
        bool do_make_token_string = false;
        switch (state)
        {
            case STATE_START:
            {
                if (c == '_' || isalpha(c))
                {
                    state = STATE_WORD;
                }
                else if (isdigit(c))
                {
                    state = STATE_NUMBER;
                }
                else if (c == '\"')
                {
                    state = STATE_STRING;
                    continue;
                }
                else
                {
                    state = STATE_SPECIAL;
                }
            } break;

            case STATE_WORD:
            {
                if (!(c == '_' || isalnum(c)))
                {
                    do_make_token = true;
                }
            } break;

            case STATE_NUMBER:
            {
                if (!isdigit(c))
                {
                    do_make_token = true;
                }
            } break;

            case STATE_STRING:
            {
                if (c == '\"')
                {
                    do_make_token_string = true;
                }
            } break;

            case STATE_SPECIAL:
            {
                // if current character is not special
                if (c == '_' || isalnum(c))
                {
                    do_make_token = true;
                }
                // if current character is special
                else
                {
                    // try putting them together
                    symbol_buffer[symbol_buffer_length] = c;
                    symbol_buffer_length++;
                    symbol_buffer[symbol_buffer_length] = 0;

                    // check if one of reserved symbols
                    bool is_reserved_symbol = false;
                    for (int i = TOK_SPECIAL_SYMBOLS_START; i < TOK_RESERVED_SYMBOL_COUNT; i++)
                    {
                        if(strcmp(symbol_buffer, reserved_symbols[i]) == 0)
                        {
                            is_reserved_symbol = true;
                        }
                    }

                    if (!is_reserved_symbol)
                    {
                        do_make_token = true;
                    }

                    symbol_buffer_length--;
                }
            } break;
        }

        if (do_make_token)
        {
            symbol_buffer[symbol_buffer_length] = 0;
            symbol_buffer_length = 0;
            Token token = make_token(symbol_buffer, line, column);
            tl_append(&token_list, token);
            state = STATE_START;
            i--; // reprocess this char
            continue;
        }
        else if (do_make_token_string)
        {
            symbol_buffer[symbol_buffer_length] = 0;
            symbol_buffer_length = 0;
            Token token = {
                .kind = TOK_STRING,
                .line = line,
                .column = column
            };
            strcpy(token.string, symbol_buffer);
            tl_append(&token_list, token);
            state = STATE_START;
            continue;
        }

        if (!isspace(c) || state == STATE_STRING)
        {
            symbol_buffer[symbol_buffer_length] = c;
            symbol_buffer_length++;
        }
    }

    symbol_buffer[symbol_buffer_length] = 0;
    Token last_token = make_token(symbol_buffer, line, column);
    tl_append(&token_list, last_token);

    Token eof = {
        .kind = TOK_EOF,
        .string = "<EOF>",
        .line = line,
        .column = column,
    };

    tl_append(&token_list, eof);

    return token_list;
}

void print_token(Token t)
{
    switch (t.kind)
    {
        case TOK_IDENT: printf("IDENT: "); break;
        case TOK_NUM: printf("NUM: "); break;
        case TOK_STRING: printf("STRING: "); break;
        default: {
            // do nothing
        }
    }

    printf("%s\n", t.string);
}
