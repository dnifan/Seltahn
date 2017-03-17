struct lexer_state {
    const char *input;
    const char *end;
    char *current;
	uint32_t line;
};
typedef struct lexer_state lexer_state;

lexer_state *lex_init(const char *input);
void lex_destroy(lexer_state *state);
void lex_fatal(const char *reason, ...);

/*
 * Perform lexical analysis.
 * @param state The state of the lexer.
 * @param token_count Pointer to an integer that will hold the total amount of tokens read.
 */
token_t **lex_run(lexer_state *state, uint32_t *token_count);
