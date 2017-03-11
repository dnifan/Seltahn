#include <stdio.h>
#include "tokens.h"
#include "lexer.h"

int main() {

    uint32_t token_count;
    lexer_state *state = lex_init("3 + 3 - 5 / 8");
    token_t **tokens = lex_run(state, &token_count);
    lex_destroy(state);

    for (int i = 0;i < token_count;i++) {
        token_t *token = tokens[i];
        printf("%10s (%02X) %08X\n", token_get_name(*token), token->type, token->param);
    }

    return 0;
}
