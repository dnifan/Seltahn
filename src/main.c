#include <stdio.h>
#include "tokens.h"
#include "lexer.h"

const char *test_code = "3 + 3";

int main() {

    lexer_state *state = lex_init(test_code);
    lex_run(state);
    lex_destroy(state);

    return 0;
}
