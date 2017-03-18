enum ast_node_type {
    EMPTY,
    TRANSLATION_UNIT,
    FUNCTION_DECLARATION,
    DECLARATOR,
    DECLARATION,
    DECLARATION_LIST,
    DECLARATION_SPECIFIER_LIST,
    TYPE_SPECIFIER,
    DIRECT_DECLARATOR,
    FUNCTION_PROTOTYPE,
    DECLARATOR_LIST,
    STORAGE_CLASS_SPECIFIER,
    TYPE_QUALIFIER,
    TYPE_QUALIFIER_LIST,
    INIT_DECLARATOR,
    INITIALIZER_LIST,
    UNARY_OPERATOR,
	COMPOUND_STATEMENT,
	FUNCTION_DEFINITION,
	STATEMENT_LIST,
    ARG_EXPRESSION_LIST,
    CAST,
    PARAMETER_LIST,
    PARAMETER_DECLARATION,
    POINTER,
    ENUM_SPECIFIER,
    ENUMERATOR_LIST,
    ENUMERATOR,

    CONDITIONAL,
    INCREMENT,
    ARE_EQUAL,
    ARE_NOT_EQUAL,
    ADD,
    SUBTRACT,
    DECREMENT,
    ARRAY_DECLARE,
    ARRAY_INDEX,
    CONSTANT_NUMBER,
    CONSTANT_STRING,
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
    SIZE_OF,
	IF_STATEMENT,
	SWITCH_STATEMENT,
	CASE_STATEMENT,
	DEFAULT_STATEMENT,
	WHILE_STATEMENT,
	FOR_STATEMENT,
	DO_STATEMENT,
    CONTINUE_STATEMENT,
    BREAK_STATEMENT,
    RETURN_STATEMENT,
    ID,
    PTR_FOLLOW,
    FIELD_FOLLOW,
    FUNCTION_CALL
};
typedef enum ast_node_type ast_node_type;

struct ast_node_t {
    ast_node_type type;
    token_t *token;
    uint32_t param;
    struct ast_node_t *left;
    struct ast_node_t *middle;
    struct ast_node_t *right;
    struct ast_node_t *postfix;
};
typedef struct ast_node_t ast_node_t;

struct ast_t {
    ast_node_t *root_node;
};
typedef struct ast_t ast_t;

ast_t *ast_create(token_t **tokens, int token_count);
void ast_fatal(token_t *token, const char *reason, ...);
const char *ast_get_type_name(ast_node_type type);
void ast_dump_start(ast_node_t *root);
token_t *ast_current();