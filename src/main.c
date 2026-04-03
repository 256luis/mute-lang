#include <stdio.h>
#include "ast.h"
#include "utils.h"
#include "token.h"

int main(int argc, char* argv[])
{
    if (argc < 2) ERROR("no input files\n");

    char* source_code_path = argv[1];
    char* source_code = read_file_to_string(source_code_path);
    if (source_code == NULL) ERROR("unable to read file\n");

    TokenList tl = tokenize(source_code);
    /* for (size_t i = 0; i < tl.count; i++) */
    /* { */
    /*     print_token(tl.tokens[i]); */
    /* } */
    parse(tl);
}
