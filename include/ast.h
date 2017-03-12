enum ast_node_type {
    TRANSLATION_UNIT,
    FUNCTION_DECLARATION,
    DECLARATION,
    TYPE_SPECIFIER,
    DIRECT_DECLARATOR
};
typedef enum ast_node_type ast_node_type;

struct ast_node_t {
    ast_node_type type;
    token_t *token;
    struct ast_node_t *left;
    struct ast_node_t *middle;
    struct ast_node_t *right;
};
typedef struct ast_node_t ast_node_t;

struct ast_t {
    ast_node_t *root_node;
};
typedef struct ast_t ast_t;

ast_t *ast_create(token_t **tokens, int token_count);
void ast_fatal(const char *reason, ...);
const char *ast_get_type_name(ast_node_type type);