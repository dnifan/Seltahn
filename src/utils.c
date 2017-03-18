#include "tokens.h"
#include "ast.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

const char *ast_translation[] = {
    "EMPTY",
    "TRANSLATION_UNIT",
    "FUNCTION_DECLARATION",
    "DECLARATOR",
    "DECLARATION",
    "DECLARATION_LIST",
    "DECLARATION_SPECIFIER_LIST",
    "TYPE_SPECIFIER",
    "DIRECT_DECLARATOR",
    "FUNCTION_PROTOTYPE",
    "DECLARATOR_LIST",
    "STORAGE_CLASS_SPECIFIER",
    "TYPE_QUALIFIER",
    "TYPE_QUALIFIER_LIST",
    "INIT_DECLARATOR",
    "INITIALIZER_LIST",
    "UNARY_OPERATOR",
	"COMPOUND_STATEMENT",
	"FUNCTION_DEFINITION",
	"STATEMENT_LIST",
    "ARG_EXPRESSION_LIST",
    "CAST",
    "PARAMETER_LIST",
    "PARAMETER_DECLARATION",
    "POINTER",

    "CONDITIONAL",
    "INCREMENT",
    "ARE_EQUAL",
    "ARE_NOT_EQUAL",
    "ADD",
    "SUBTRACT",
    "DECREMENT",
    "ARRAY_DECLARE",
    "ARRAY_INDEX",
    "CONSTANT_NUMBER",
    "CONSTANT_STRING",
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
	"DO_STATEMENT",
    "CONTINUE_STATEMENT",
    "BREAK_STATEMENT",
    "RETURN_STATEMENT",
    "SYMBOL_REF",
    "PTR_FOLLOW",
    "FIELD_FOLLOW",
    "FUNCTION_CALL"
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

    printf("  n%p [label=\"%s\"];\n", node, ast_get_type_name(node->type));

    if (node->left != NULL)
        printf("  n%p -- n%p [label=%i];\n", node, node->left, node->left->param);
    if (node->middle != NULL)
        printf("  n%p -- n%p [label=%i];\n", node, node->middle, node->middle->param);
    if (node->right != NULL)
        printf("  n%p -- n%p [label=%i];\n", node, node->right, node->right->param);
    if (node->postfix!= NULL)
        printf("  n%p -- n%p [label=\"Postfix\"];\n", node, node->postfix);

    ast_dump(node->left);
    ast_dump(node->middle);
    ast_dump(node->right);
    ast_dump(node->postfix);
}

void ast_dump_start(ast_node_t *root) {
    printf("Copy & paste this into a GraphViz tool (e.g. http://www.webgraphviz.com/)\n\n");
    printf("graph ast {\n");
    ast_dump(root);
    printf("}\n\n\n");
}

const char *ast_get_type_name(ast_node_type type) {
    return ast_translation[type];
}