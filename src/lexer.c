#include "tokens.h"
#include "lexer.h"
#include "list.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <varargs.h>

uint32_t is_end_of_token(char ch) {
    return (ch == SEMICOLON || iswspace(ch));
}

uint32_t read_number(lexer_state *state) {
    uint32_t result;

    char buf[32];
    memset(buf, 0, 32);
    char *ptr = &buf;
    while (!is_end_of_token(*state->current) && state->current < state->end) {
        *ptr = *state->current;
        state->current++;

        if (ptr == (&buf + sizeof(buf)-1))
            lex_fatal("Buffer overflow in read_number()");
    }

    return atoi(buf);
}

uint32_t eat_whitespace(lexer_state *state) {
    uint32_t counter = 0;
    while (iswspace(*state->current) && state->current < state->end) {
        state->current++;
        counter++;
    }
    return counter;
}

uint8_t is_operator(char ch) {
    return ch == '+' ||
        ch == '-' ||
        ch == '*' ||
        ch == '/';
}

operator_type_t get_operator(char ch) {
    switch (ch) {
    case '+': return PLUS;
    case '-': return MINUS;
    case '*': return ASTERISK;
    case '/': return SLASH;
    default:
        lex_fatal("%c is not an operator.", ch);
    }
}

token_t read_token(lexer_state *state) {
    token_t token;
    token.param.number = 0;

    if (isdigit(*state->current)) {
        // It must be a number, read it.
        token.type = NUMBER;
        token.param.number = read_number(state);
    }
    else if (iswspace(*state->current)) {
        token.type = WHITESPACE;
        token.param.number = eat_whitespace(state);;
    }
    else if (is_operator(*state->current)) {
        token.type = OPERATOR;
        token.param.number = (uint32_t)get_operator(*state->current);
        state->current++;
    }
    else {
        lex_fatal("Unknown token '%c'!", *state->current);
    }

    return token;
}

token_t *lex_run(lexer_state *state) {
    linked_list *list = list_new();

    while (state->current < state->end) {
        token_t token = read_token(state);
        printf("%i %08X\n", token.type, token.param);
    }

    return NULL;
}

lexer_state *lex_init(const char * input)
{
    lexer_state *state = (lexer_state*)malloc(sizeof(lexer_state));

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
    printf("Fatal error: ");
    vprintf(reason, ap);
    printf("\n\n");
    va_end(ap);

    exit(0);
}