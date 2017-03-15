enum ast_node_type {
    TRANSLATION_UNIT,
    FUNCTION_DECLARATION,
    DECLARATION,
    DECLARATION_SPECIFIER_LIST,
    TYPE_SPECIFIER,
    DIRECT_DECLARATOR,
    FUNCTION_PROTOTYPE,
    DECLARATOR_LIST,
    STORAGE_CLASS_SPECIFIER,
    TYPE_QUALIFIER,
    INIT_DECLARATOR,
    UNARY_OPERATOR,

    
    CONDITIONAL,
    INCREMENT,
    ARE_EQUAL,
    ARE_NOT_EQUAL,
    ADD,
    SUBTRACT,
    DECREMENT,
    ARRAY_DECLARE,
    ARRAY_INDEX,
    CONSTANT,
    ASSIGNMENT,
    ASSIGNMENT_OPERATOR,
    BITSHIFT_LEFT,
    BITSHIFT_RIGHT,
    IS_GREATER_THAN,
    IS_LESS_THAN,
    IS_GREATER_THAN_OR_EQUAL,
    IS_LESS_THAN_OR_EQUAL,
    BITWISE_AND,
    BITWISE_OR,
    EXCLUSIVE_OR,
    LOGICAL_AND,
    LOGICAL_OR,
    MULTIPLY,
    DIVIDE,
    MODULO,
    SIZE_OF
};
typedef enum ast_node_type ast_node_type;

union ast_parameter {
    uint32_t number;

};

struct ast_node_t {
    ast_node_type type;
    token_t *token;
    union ast_parameter param;
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
void ast_dump_start(ast_node_t *root);