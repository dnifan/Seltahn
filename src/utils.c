#include "tokens.h"
#include "ast.h"

#include <stdarg.h>

void ast_fatal(const char *reason, ...) {
    va_list ap;
    va_start(ap, reason);
    printf("ast_fatal: ");
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
    switch (type) {
    case TRANSLATION_UNIT: return "TRANSLATION_UNIT";
    case DECLARATION: return "DECLARATION";
    case TYPE_SPECIFIER: return "TYPE_SPECIFIER";
    case DIRECT_DECLARATOR: return "DIRECT_DECLARATOR";
    case DECLARATOR_LIST: return "DECLARATOR_LIST";
    case TYPE_QUALIFIER: return "TYPE_QUALIFIER";
    case DECLARATION_SPECIFIER_LIST: return "DECLARATION_SPECIFIER_LIST";
    case STORAGE_CLASS_SPECIFIER: return "STORAGE_CLASS_SPECIFIER";
    case UNARY_OPERATOR: return "UNARY_OPERATOR";
    case INIT_DECLARATOR: return "INIT_DECLARATOR";

    case CONDITIONAL: return "CONDITIONAL";
    case INCREMENT: return "INCREMENT";
    case DECREMENT:return "DECREMENT";
    case ARRAY_INDEX:return "ARRAY_INDEX";
    case CONSTANT:return "CONSTANT";
    case ARE_EQUAL: return "ARE_EQUAL";
    case ARE_NOT_EQUAL: return "ARE_NOT_EQUAL";
    case ADD: return "ADD";
    case SUBTRACT: return "SUBTRACT";
    case BITSHIFT_LEFT: return "BITSHIFT_LEFT";
    case BITSHIFT_RIGHT: return "BITSHIFT_RIGHT";
    case IS_GREATER_THAN: return "IS_GREATER_THAN";
    case IS_LESS_THAN: return "IS_LESS_THAN";
    case IS_GREATER_THAN_OR_EQUAL: return "IS_GREATER_THAN_OR_EQUAL";
    case IS_LESS_THAN_OR_EQUAL: return "IS_LESS_THAN_OR_EQUAL";
    case BITWISE_AND: return "BITWISE_AND";
    case BITWISE_OR: return "BITWISE_OR";
    case LOGICAL_AND: return "LOGICAL_AND";
    case LOGICAL_OR: return "LOGICAL_OR";
    case EXCLUSIVE_OR: return "EXCLUSIVE_OR";
    case MULTIPLY: return "MULTIPLY";
    case DIVIDE: return "DIVIDE";
    case MODULO: return "MODULO";
    case ARRAY_DECLARE: return "ARRAY_DECLARE";
    case SIZE_OF: return "SIZE_OF";
    default: return "Unknown";
    }
}