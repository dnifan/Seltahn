#include <stdint.h>

/*!
 * Defines a token of the C language.
 */
enum token_type_t {
    SYMBOL,
    NUMBER,
    WHITESPACE,
    OPERATOR,

    SEMICOLON,

    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
};
typedef enum token_type_t token_type_t;

enum operator_type_t {
    NONE,
    PLUS,
    MINUS,
    ASTERISK,
    SLASH
};
typedef enum operator_type_t operator_type_t;

union token_parameter {
    uint32_t number;
    void *pointer;
    const char *string;
};

struct token_t {
    token_type_t type;
    union token_parameter param;
};
typedef struct token_t token_t;

const char *token_get_name(token_t token);