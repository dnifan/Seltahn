#include "tokens.h"

const char *token_translation[] = {
    "SYMBOL",
    "NUMBER",
    "WHITESPACE",
    "OPERATOR",

    "SEMICOLON",

    "LPAREN",
    "RPAREN",
    "LBRACE",
    "RBRACE",
};

const char *token_get_name(token_t token) {
    return token_translation[token.type];
}