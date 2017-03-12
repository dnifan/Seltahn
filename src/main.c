#include <stdio.h>
#include "tokens.h"
#include "lexer.h"
#include "ast.h"

const char *example = "int main()\
{\
    char  line[100];\
    int   total;\
    int   item;\
\
    total = 0;\
    while (1) {\
        printf(\"Enter # to add \\n\");\
        printf(\"  or 0 to stop:\");\
\
        fgets(line, sizeof(line), stdin);\
        sscanf(line, \"%d\", &item);\
\
        if (item == 0)\
            break;\
\
        total += item;\
        printf(\"Total: %d\\n\", total);\
    }\
    printf(\"Final total %d\\n\", total);\
    return (0);\
}";

void tokens_debug(token_t **tokens, int token_count) {
    for (int i = 0;i < token_count;i++) {
        token_t *token = tokens[i];

        if (token->type == WHITESPACE)
            continue;

        printf("%10s (%02X) %08X", token_get_name(*token), token->type, token->param);

        if (token->type == SYMBOL || token->type == STRING) {
            printf(" (%s)", token->param.string);
        }

        printf("\n");
    }
}

int main() {

    uint32_t token_count;
    lexer_state *state = lex_init("int ((a))[];");
    token_t **tokens = lex_run(state, &token_count);
    ast_t *ast = ast_create(tokens, token_count);
    
    lex_destroy(state);
    return 0;
}
