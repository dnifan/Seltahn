struct lexer_state {
    const char *input;
    const char *end;
    char *current;
};
typedef struct lexer_state lexer_state;

lexer_state *lex_init(const char *input);
void lex_destroy(lexer_state *state);
void lex_fatal(const char *reason, ...);
token_t *lex_run(lexer_state *state);
