#include "tokens.h"
#include "ast.h"

#include <varargs.h>
#include <stdarg.h>
#include <stdio.h>

token_t *current;
uint32_t total_tokens, token_index;
token_t **tokens;

ast_node_t *declarator();
void ast_dump_start(ast_node_t *root);

ast_node_t *new_node() {
    ast_node_t *node = (ast_node_t*)malloc(sizeof(ast_node_t));
    memset(node, 0, sizeof(ast_node_t));
    return node;
}

void next() {
    if (token_index >= total_tokens)
        ast_fatal("unexpected end of stream");
    current = tokens[token_index++];
}

void expect(token_type_t tok) {
    if (current->type != tok)
        ast_fatal("Expected %s.", token_get_name_by_type(tok));
}

ast_node_t *type_specifier() {
    if (current->type == VOID ||
        current->type == INT ||
        current->type == CHAR ||
        current->type == SHORT ||
        current->type == LONG ||
        current->type == FLOAT ||
        current->type == DOUBLE ||
        current->type == SIGNED ||
        current->type == UNSIGNED ||
        current->type == SYMBOL) {

        ast_node_t *res = new_node();
        res->token = current;
        res->type = TYPE_SPECIFIER;
        next();
        return res;
    }

    return NULL;
}

ast_node_t *declaration_specifiers() {
    ast_node_t *ts = type_specifier();
    if (ts == NULL) {
        // check storage_class_specifier
        // check type_qualifier
    }
    return ts;
}

ast_node_t *constant_expression() {
    return NULL; // TODO
}

ast_node_t *parameter_type_list() {
    return NULL; // TODO
}

ast_node_t *direct_declarator() {
    ast_node_t *res;
    if (current->type == SYMBOL) {
        res = new_node();
        res->type = DIRECT_DECLARATOR;
        next();
    }
    else if (current->type == LPAREN) {
        next(); // (
        res = declarator(tokens);
        expect(RPAREN);
        next(); // )
    }

    if (current->type == LBRACK) {
        // array declaration
        next(); // [
        ast_node_t *cs = constant_expression();
        expect(RBRACK);
        next(); // ]
    }
    else if (current->type == LPAREN) {
        // function prototype.
        next(); // (
        ast_node_t *ptl = parameter_type_list();
        expect(RPAREN);
        next(); // )
    }
    
    return res;
}

ast_node_t *declarator() {
    return direct_declarator();
}

ast_node_t *init_declarator() {
    return declarator();
}

ast_node_t *init_declarator_list() {
    return init_declarator();
}

ast_node_t *declaration() {
    ast_node_t *d = new_node();

    d->type = DECLARATION;
    d->left = declaration_specifiers();
    if (d->left == NULL)
        return NULL;
    
    // Apparently, C89 allows declaring a variable without a declarator list.
    // So, "int;" is allowed. :/
    d->right = init_declarator_list();

    expect(SEMICOLON);
    return d;
}

ast_node_t *external_declaration() {
    ast_node_t *d = declaration();
    if (d == NULL) {
        // function declaration
        ast_fatal("func decl not supported yet");
    }
    return d;
}

ast_node_t *translation_unit() {
    ast_node_t *thisNode = new_node();
    thisNode->type = TRANSLATION_UNIT;
    thisNode->middle = external_declaration();
    return thisNode;
}

ast_t *ast_create(token_t **t, int token_count) {
    tokens = t;
    token_index = 0;
    total_tokens = token_count;

    next();
    ast_node_t *root_node = translation_unit();

    ast_dump_start(root_node);
}

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
        printf("  n%08X -- n%08X;\n", node, node->left);
    if (node->middle != NULL)
        printf("  n%08X -- n%08X;\n", node, node->middle);
    if (node->right != NULL)
        printf("  n%08X -- n%08X;\n", node, node->right);

    ast_dump(node->left);
    ast_dump(node->middle);
    ast_dump(node->right);

}

void ast_dump_start(ast_node_t *root) {
    printf("Copy & paste this into a GraphViz tool (http://www.webgraphviz.com/)\n\n");
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
        default: return "Unknown";
    }
}