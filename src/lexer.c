#include "tokens.h"
#include "lexer.h"
#include "list.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <varargs.h>

uint32_t read_number(lexer_state *state) {
    uint32_t result;

    char buf[32];
    memset(buf, 0, 32);
    char *ptr = &buf;
    while (isdigit(*state->current) && state->current < state->end) {
        *ptr++ = *state->current++;
        if (ptr == (&buf + sizeof(buf)-1))
            lex_fatal("Buffer overflow in read_number()");
    }

    return atoi(buf);
}

const char *read_literal(lexer_state *state) {
    char *result = (char*)malloc(1024); // TODO: check for buffer overflow
    memset(result, 0, 1024);
    char *ptr = result;
    
    while (state->current < state->end && (isalnum(*state->current) || *state->current == '_')) {
        *ptr++ = *state->current++;
    }
    return (const char *)result;
}

const char *read_string(lexer_state *state) {
    char *result = (char*)malloc(1024); // TODO: check for buffer overflow
    memset(result, 0, 1024);
    char *ptr = result;
    while (*state->current != '"' && state->current < state->end) {
        *ptr++ = *state->current++;
    }
    state->current++;
    return (const char *)result;
}

uint32_t eat_whitespace(lexer_state *state) {
    uint32_t counter = 0;
    while (iswspace(*state->current) && state->current < state->end) {
        state->current++;
        counter++;
    }
    return counter;
}

token_t *read_token(lexer_state *state) {
    token_t *token = (token_t*)malloc(sizeof(token_t));
    token->param.number = 0;

	if (*state->current == '\n')
		state->line++;

    if (isdigit(*state->current)) {
        // It must be a number, read it.
        token->type = NUMBER;
        token->param.number = read_number(state);
    }
    else if (iswspace(*state->current)) {
		token->type = WHITESPACE;
        token->param.number = eat_whitespace(state);;
    }
    else if (*state->current == '"') {
        state->current++;
        token->type = STRING;
        token->param.string = read_string(state);
    }
    else if (*state->current == ';') { token->type = SEMICOLON; state->current++; }
    else if (*state->current == '(') { token->type = LPAREN; state->current++; }
    else if (*state->current == ')') { token->type = RPAREN; state->current++; }
    else if (*state->current == '{') { token->type = LBRACE; state->current++; }
    else if (*state->current == '}') { token->type = RBRACE; state->current++; }
    else if (*state->current == '[') { token->type = LBRACK; state->current++; }
    else if (*state->current == ']') { token->type = RBRACK; state->current++; }
    else if (*state->current == ',') { token->type = COMMA; state->current++; }
    else if (*state->current == '?') { token->type = QUESTION; state->current++; }
    else if (*state->current == ':') { token->type = COLON; state->current++; }
    else if (*state->current == '~') { token->type = TILDE; state->current++; }
    else if (*state->current == '>') {
        state->current++;

        if (*state->current == '>') { 
            state->current++;
            if (*state->current == '=') { token->type = RSH_ASS; state->current++; }  // >>=
            else { token->type = OP_RSH; }                                            // >>
        }
        else if (*state->current == '=') { token->type = OP_GTE; state->current++; }  // >=
        else { token->type = OP_GT; }                                                 // >
    }
    else if (*state->current == '<') {
        state->current++;

        if (*state->current == '<') {
            state->current++;
            if (*state->current == '=') { token->type = LSH_ASS; state->current++; }  // <<=
            else { token->type = OP_LSH; }                                            // <<
        }
        else if (*state->current == '=') { token->type = OP_LTE; state->current++; }  // <=
        else { token->type = OP_LT; }                                                 // <
    }
    else if (*state->current == '=') {
        state->current++;

        if (*state->current == '=') { token->type = OP_EQUALS; state->current++; }    // ==
        else { token->type = ASSIGN; }                                                // =
    }
    else if (*state->current == '+') {
        state->current++;

        if (*state->current == '+') { token->type = OP_INC; state->current++; }       // ++
        else if (*state->current == '=') { token->type = ADD_ASS; state->current++; } // +=
        else { token->type = PLUS; }                                                  // +
    }
    else if (*state->current == '-') {
        state->current++;

        if (*state->current == '-') { token->type = OP_DEC; state->current++; }       // --
        else if (*state->current == '=') { token->type = SUB_ASS; state->current++; } // -=
        else if (*state->current == '>') { token->type = OP_PTR; state->current++; }  // ->
        else { token->type = MINUS; }                                                 // -
    }
    else if (*state->current == '&') {
        state->current++;

        if (*state->current == '&') { token->type = OP_AND; state->current++; }       // &&
        else if (*state->current == '=') { token->type = AND_ASS; state->current++; } // &=
        else { token->type = AMPERS; }                                                // &
    }
    else if (*state->current == '|') {
        state->current++;

        if (*state->current == '|') { token->type = OP_OR; state->current++; }        // ||
        else if (*state->current == '=') { token->type = OR_ASS; state->current++; }  // |=
        else { token->type = PIPE; }                                                  // |
    }
    else if (*state->current == '^') {
        state->current++;

        if (*state->current == '=') { token->type = XOR_ASS; state->current++; }      // ^=
        else { token->type = XOR; }                                                   // ^
    }
    else if (*state->current == '!') {
        state->current++;

        if (*state->current == '=') { token->type = OP_NEQ; state->current++; }       // !=
        else { token->type = BANG; }                                                  // ^
    }
    else if (*state->current == '*') {
        state->current++;

        if (*state->current == '=') { token->type = MUL_ASS; state->current++; }      // *=
        else { token->type = ASTERISK; }                                              // *
    }
    else if (*state->current == '/') {
        state->current++;

        if (*state->current == '=') { token->type = DIV_ASS; state->current++; }      // /=
        else { token->type = SLASH; }                                                 // /
    }
    else if (*state->current == '%') {
        state->current++;

        if (*state->current == '=') { token->type = MOD_ASS; state->current++; }      // %=
        else { token->type = PERCENT; }                                               // %
    }
    else if (*state->current == '.') {
        state->current++;

        if (*state->current == '.' && *(state->current + 1) == '.') { token->type = ELLIPS; state->current += 2; } // ...
        else { token->type = PERIOD; }
    }
    else if (isalnum(*state->current)) {
        // Check whether the literal is a C89 keyword.
        const char *lit = read_literal(state);

        if (strcmp(lit, "break") == 0) { token->type = BREAK; }
        else if (strcmp(lit, "case") == 0) { token->type = CASE; }
        else if (strcmp(lit, "char") == 0) { token->type = CHAR; }
        else if (strcmp(lit, "const") == 0) { token->type = CONST; }
        else if (strcmp(lit, "continue") == 0) { token->type = CONTINUE; }
        else if (strcmp(lit, "default") == 0) { token->type = DEFAULT; }
        else if (strcmp(lit, "do") == 0) { token->type = DO; }
        else if (strcmp(lit, "double") == 0) { token->type = DOUBLE; }
        else if (strcmp(lit, "else") == 0) { token->type = ELSE; }
        else if (strcmp(lit, "enum") == 0) { token->type = ENUM; }
        else if (strcmp(lit, "extern") == 0) { token->type = EXTERN; }
        else if (strcmp(lit, "float") == 0) { token->type = FLOAT; }
        else if (strcmp(lit, "for") == 0) { token->type = FOR; }
        else if (strcmp(lit, "goto") == 0) { token->type = GOTO; }
        else if (strcmp(lit, "if") == 0) { token->type = IF; }
        else if (strcmp(lit, "int") == 0) { token->type = INT; }
        else if (strcmp(lit, "long") == 0) { token->type = LONG; }
        else if (strcmp(lit, "register") == 0) { token->type = REGISTER; }
        else if (strcmp(lit, "return") == 0) { token->type = RETURN; }
        else if (strcmp(lit, "short") == 0) { token->type = SHORT; }
        else if (strcmp(lit, "signed") == 0) { token->type = SIGNED; }
        else if (strcmp(lit, "sizeof") == 0) { token->type = SIZEOF; }
        else if (strcmp(lit, "static") == 0) { token->type = STATIC; }
        else if (strcmp(lit, "struct") == 0) { token->type = STRUCT; }
        else if (strcmp(lit, "switch") == 0) { token->type = SWITCH; }
        else if (strcmp(lit, "typedef") == 0) { token->type = TYPEDEF; }
        else if (strcmp(lit, "union") == 0) { token->type = UNION; }
        else if (strcmp(lit, "unsigned") == 0) { token->type = UNSIGNED; }
        else if (strcmp(lit, "void") == 0) { token->type = VOID; }
        else if (strcmp(lit, "volatile") == 0) { token->type = VOLATILE; }
        else if (strcmp(lit, "while") == 0) { token->type = WHILE; }
        else {
            token->type = SYMBOL;
            token->param.string = lit;
        }
    }
    else {
        lex_fatal("Unknown token '%c'!", *state->current);
    }

	token->line = state->line;
    return token;
}

token_t **lex_run(lexer_state *state, uint32_t *token_count) {
    linked_list *list = list_new();

    while (state->current < state->end) {
        token_t *token = read_token(state);
        if (token->type == WHITESPACE)
            continue;
        list_add(list, token);
    }
    
    *token_count = list->length;
    return list_toarray(list);
}

lexer_state *lex_init(const char * input)
{
    lexer_state *state = (lexer_state*)malloc(sizeof(lexer_state));

	state->line = 1;
    state->input = (const char *)malloc(strlen(input) + 1);
    strcpy((char*)state->input, input);
    state->current = state->input;
    state->end = state->input + strlen(state->input);

    return state;
}

void lex_destroy(lexer_state *state) {
    free(state);
}

void lex_fatal(const char *reason, ...) {
    va_list ap;
    va_start(ap, reason);
    printf("lex_fatal: ");
    vprintf(reason, ap);
    printf("\n\n");
    va_end(ap);

    exit(0);
}