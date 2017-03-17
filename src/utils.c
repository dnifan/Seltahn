#include "tokens.h"
#include "ast.h"

#include <stdarg.h>

const char *ast_translation[] = {
    "TRANSLATION_UNIT",
    "FUNCTION_DECLARATION",
    "DECLARATION",
    "DECLARATION_SPECIFIER_LIST",
    "TYPE_SPECIFIER",
    "DIRECT_DECLARATOR",
    "FUNCTION_PROTOTYPE",
    "DECLARATOR_LIST",
    "STORAGE_CLASS_SPECIFIER",
    "TYPE_QUALIFIER",
    "INIT_DECLARATOR",
    "UNARY_OPERATOR",
	"COMPOUND_STATEMENT",
	"FUNCTION_DEFINITION",
	"STATEMENT_LIST",



    "CONDITIONAL",
    "INCREMENT",
    "ARE_EQUAL",
    "ARE_NOT_EQUAL",
    "ADD",
    "SUBTRACT",
    "DECREMENT",
    "ARRAY_DECLARE",
    "ARRAY_INDEX",
    "CONSTANT",
    "ASSIGNMENT",
    "ASSIGNMENT_OPERATOR",
    "BITSHIFT_LEFT",
    "BITSHIFT_RIGHT",
    "IS_GREATER_THAN",
    "IS_LESS_THAN",
    "IS_GREATER_THAN_OR_EQUAL",
    "IS_LESS_THAN_OR_EQUAL",
    "BITWISE_AND",
    "BITWISE_OR",
    "EXCLUSIVE_OR",
    "LOGICAL_AND",
    "LOGICAL_OR",
    "MULTIPLY",
    "DIVIDE",
    "MODULO",
    "SIZE_OF",
	"IF_STATEMENT",
	"SWITCH_STATEMENT",
	"CASE_STATEMENT",
	"DEFAULT_STATEMENT",
	"WHILE_STATEMENT",
	"FOR_STATEMENT",
	"DO_STATEMENT"
};

void ast_fatal(token_t *token, const char *reason, ...) {
    va_list ap;
    va_start(ap, reason);
    printf("ast_fatal: Line %i: ", token->line);
    vprintf(reason, ap);
    printf("\n\n");
    va_end(ap);

    exit(0);
}

void ast_dump(ast_node_t *node) {
    if (node == NULL)
        return;

    printf("  n%08X [label=\"%s\"];\n", node, ast_get_type_name(node->type));

    if (node->left != NULL)
        printf("  n%08X -- n%08X [label=%i];\n", node, node->left, node->left->param);
    if (node->middle != NULL)
        printf("  n%08X -- n%08X [label=%i];\n", node, node->middle, node->middle->param);
    if (node->right != NULL)
        printf("  n%08X -- n%08X [label=%i];\n", node, node->right, node->right->param);

    ast_dump(node->left);
    ast_dump(node->middle);
    ast_dump(node->right);

}

void ast_dump_start(ast_node_t *root) {
    printf("Copy & paste this into a GraphViz tool (e.g. http://www.webgraphviz.com/)\n\n");
    printf("graph ast {\n");
    ast_dump(root);
    printf("}\n\n\n");
}

// TODO: put in table
const char *ast_get_type_name(ast_node_type type) {
    return ast_translation[type];
}