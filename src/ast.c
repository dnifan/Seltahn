#include "tokens.h"
#include "ast.h"

#include <varargs.h>
#include <stdarg.h>
#include <stdio.h>

token_t *current;
uint32_t total_tokens, token_index;
token_t **tokens;

ast_node_t *declarator();
ast_node_t *expression();
ast_node_t *cast_expression();
ast_node_t *conditional_expression();
void ast_dump_start(ast_node_t *root);

ast_node_t *new_node(ast_node_type type) {
    ast_node_t *node = (ast_node_t*)malloc(sizeof(ast_node_t));
    memset(node, 0, sizeof(ast_node_t));
    node->type = type;
    return node;
}

uint8_t is_eof() {
    return token_index >= total_tokens;
}

void next() {
    if (is_eof())
        ast_fatal("unexpected end of stream");
    current = tokens[token_index++];
}

/**
 * @brief Checks if the current token is of the specified type, and advances the token if true.
 * @returns Whether the current token is of the specified type.
 */
uint8_t accept(token_type_t tok) {
    if (current->type == tok) {
        next();
        return 1;
    }

    return 0;
}

void expect(token_type_t tok) {
    if (current->type != tok)
        ast_fatal("Expected %s.", token_get_name_by_type(tok));

    if (!is_eof())
        next();
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
        current->type == UNSIGNED /*||
        current->type == SYMBOL*/) {

        ast_node_t *res = new_node(TYPE_SPECIFIER);
        res->token = current;
        next();
        return res;
    }

    return NULL;
}

ast_node_t *storage_class_specifier() {
    if (current->type == TYPEDEF ||
        current->type == EXTERN ||
        current->type == STATIC ||
        current->type == REGISTER) {

        ast_node_t *res = new_node(STORAGE_CLASS_SPECIFIER);
        next();
        return res;
    }

    return NULL;
}

ast_node_t *type_qualifier() {
    if (current->type == CONST ||
        current->type == VOLATILE) {
        ast_node_t *res = new_node(TYPE_QUALIFIER);
        next();
        return res;
    }

    return NULL;
}

ast_node_t *declaration_specifiers() {
    ast_node_t *left = storage_class_specifier();
    if (left == NULL) {
        left = type_specifier();
        if (left == NULL) {
            left = type_qualifier();
            if (left == NULL) {
                // in the end, return NULL to signal the end of the declaration specifier list.
                return NULL;
            }
        }
    }
    
    ast_node_t *node = new_node(DECLARATION_SPECIFIER_LIST);
    node->left = left;
    node->right = declaration_specifiers();
    return node;
}

ast_node_t *constant_expression() {
    return conditional_expression();
}

ast_node_t *parameter_type_list() {
    return NULL; // TODO
}

ast_node_t *direct_declarator() {
    ast_node_t *res = NULL;
    if (current->type == SYMBOL) {
        res = new_node(DIRECT_DECLARATOR);
        next();
    }
    else if (current->type == LPAREN) {
        next(); // (
        res = declarator(tokens);
        expect(RPAREN);
    }

    if (current->type == LBRACK) {
        // array declaration
        next(); // [
        res = new_node(ARRAY_DECLARE);
        res->left = constant_expression();
        expect(RBRACK);
        return res;
    }
    else if (current->type == LPAREN) {
        // function prototype.
        next(); // (
        ast_node_t *ptl = parameter_type_list();
        expect(RPAREN);
    }
    
    return res;
}

ast_node_t *declarator() {
    return direct_declarator();
}

ast_node_t *primary_expression() {
    if (current->type == NUMBER) {
        ast_node_t *result = new_node(CONSTANT);
        result->param = current->param.number;
        next();
        return result;
    }
    else if (accept(LPAREN)) {
        ast_node_t *result = expression();
        expect(RPAREN);
        return result;
    }
    else {
        return NULL;
    }
}

ast_node_t *postfix_expression() {  // a[123]

    return primary_expression();

    /*ast_node_t *n = primary_expression();
    
    if (accept(LBRACK)) {
        ast_node_t *result = new_node(ARRAY_INDEX);
        result->
    }*/
    
    /*if (current->type == NUMBER) {
        ast_node_t *n = new_node(CONSTANT);
        n->param = current->param.number;
        next();
        return n;
    }*/
}

ast_node_t *unary_operator() {
    if (current->type == AMPERS ||
        current->type == ASTERISK ||
        current->type == PLUS ||
        current->type == MINUS ||
        current->type == TILDE ||
        current->type == BANG) {
        ast_node_t *node = new_node(UNARY_OPERATOR);
        node->token = current;
        return node;
    }

    return NULL;
}

ast_node_t *unary_expression() {
    if (accept(OP_INC)) {
        ast_node_t *n = new_node(INCREMENT);
        n->middle = unary_expression();
        return n;
    }
    else if (accept(OP_DEC)) {
        ast_node_t *n = new_node(DECREMENT);
        n->middle = unary_expression();
        return n;
    }
    else if (accept(SIZEOF)) {
        ast_node_t *n = new_node(SIZE_OF);
        
        if (accept(LPAREN)) {
            ast_fatal("sizeof(type_name) not supported yet.");
        }

        n->left = unary_expression();
        return n;
    }
    else {
        ast_node_t *n = unary_operator();
        if (n == NULL) {
            n = postfix_expression();
            if (n == NULL) {
                return NULL;
            }
        }
        else {
            // unary operator
            n->left = cast_expression();
        }
        return n;
    }
}

ast_node_t *cast_expression() {
    // TODO: make casting code
    return unary_expression();
}

ast_node_t *multiplicative_expression() {
    ast_node_t *n = cast_expression();
    if (accept(ASTERISK)) {
        ast_node_t *result = new_node(MULTIPLY);
        result->left = n;
        result->right = multiplicative_expression();
        return result;
    }
    else if (accept(SLASH)) {
        ast_node_t *result = new_node(DIVIDE);
        result->left = n;
        result->right = multiplicative_expression();
        return result;
    }
    else if (accept(PERCENT)) {
        ast_node_t *result = new_node(MODULO);
        result->left = n;
        result->right = multiplicative_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *additive_expression() {
    ast_node_t *n = multiplicative_expression();
    if (accept(PLUS)) {
        ast_node_t *result = new_node(ADD);
        result->left = n;
        result->right = additive_expression();
        return result;
    }
    else if (accept(MINUS)) {
        ast_node_t *result = new_node(SUBTRACT);
        result->left = n;
        result->right = additive_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *shift_expression() {
    ast_node_t *n = additive_expression();
    if (accept(OP_LSH)) {
        ast_node_t *result = new_node(BITSHIFT_LEFT);
        result->left = n;
        result->right = shift_expression();
        return result;
    }
    else if (accept(OP_RSH)) {
        ast_node_t *result = new_node(BITSHIFT_RIGHT);
        result->left = n;
        result->right = shift_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *relational_expression() {
    ast_node_t *n = shift_expression();
    if (accept(OP_LT)) {
        ast_node_t *result = new_node(IS_LESS_THAN);
        result->left = n;
        result->right = relational_expression();
        return result;
    }
    else if (accept(OP_GT)) {
        ast_node_t *result = new_node(IS_GREATER_THAN);
        result->left = n;
        result->right = relational_expression();
        return result;
    }
    else if (accept(OP_LTE)) {
        ast_node_t *result = new_node(IS_LESS_THAN_OR_EQUAL);
        result->left = n;
        result->right = relational_expression();
        return result;
    }
    else if (accept(OP_GTE)) {
        ast_node_t *result = new_node(IS_GREATER_THAN_OR_EQUAL);
        result->left = n;
        result->right = relational_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *equality_expression() {
    ast_node_t *n = relational_expression();
    if (accept(OP_EQUALS)) {
        ast_node_t *result = new_node(ARE_EQUAL);
        result->left = n;
        result->right = equality_expression();
        return result;
    }
    else if (accept(OP_NEQ)) {
        ast_node_t *result = new_node(ARE_NOT_EQUAL);
        result->left = n;
        result->right = equality_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *and_expression() {
    ast_node_t *n = equality_expression();
    if (accept(AMPERS)) {
        ast_node_t *result = new_node(BITWISE_AND);
        result->left = n;
        result->right = and_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *exclusive_or_expression() {
    ast_node_t *n = and_expression();
    if (accept(XOR)) {
        ast_node_t *result = new_node(EXCLUSIVE_OR);
        result->left = n;
        result->right = exclusive_or_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *inclusive_or_expression() {
    ast_node_t *n = exclusive_or_expression();
    if (accept(PIPE)) {
        ast_node_t *result = new_node(BITWISE_OR);
        result->left = n;
        result->right = inclusive_or_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *logical_and_expression() {
    ast_node_t *n = inclusive_or_expression();
    if (accept(OP_AND)) {
        ast_node_t *result = new_node(LOGICAL_AND);
        result->left = n;
        result->right = logical_and_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *logical_or_expression() {
    ast_node_t *n = logical_and_expression();
    if (accept(OP_OR)) {
        ast_node_t *result = new_node(LOGICAL_OR);
        result->left = n;
        result->right = logical_or_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *conditional_expression() {
    ast_node_t *n = logical_or_expression();
    
    // a ? b : c
    if (accept(QUESTION)) {
        ast_node_t *result = new_node(CONDITIONAL);
        result->left = n;
        result->middle = expression();
        expect(COLON);
        result->right = conditional_expression();
        return result;
    }
    else {
        return n;
    }
}

ast_node_t *assignment_operator() {
    if (current->type == ASSIGN ||
        current->type == MUL_ASS ||
        current->type == DIV_ASS ||
        current->type == MOD_ASS ||
        current->type == ADD_ASS ||
        current->type == SUB_ASS ||
        current->type == LSH_ASS ||
        current->type == RSH_ASS ||
        current->type == AND_ASS ||
        current->type == XOR_ASS ||
        current->type == OR_ASS) {
        ast_node_t *result = new_node(ASSIGNMENT_OPERATOR);
        result->token = current;
        next();
        return result;
    }

    return NULL;
}

ast_node_t *assignment_expression() {
    ast_node_t *n = conditional_expression();

    if (n == NULL) {
        ast_node_t *left = unary_expression();
        if (left == NULL)
            return NULL;

        ast_node_t *middle = assignment_operator();
        if (middle == NULL)
            return NULL;

        ast_node_t *right = assignment_expression();
        if (right == NULL)
            return NULL;

        n = new_node(ASSIGNMENT);
        n->left = left;
        n->middle = middle;
        n->right = right;
        return n;
    }
}

ast_node_t *expression() {
    ast_node_t *n = assignment_expression();
    if (accept(COMMA)) {
        n->right = expression();
    }
    return n;
}

ast_node_t *initializer() {
    return conditional_expression();
}

ast_node_t *init_declarator() {
    ast_node_t *node = new_node(INIT_DECLARATOR);
    node->left = declarator();
    
    if (accept(ASSIGN)) {
        node->right = initializer();
    }

    return node;
}

ast_node_t *init_declarator_list() {
    ast_node_t *left = init_declarator();
    if (left == NULL) {
        return NULL;
    }

    ast_node_t *node = new_node(DECLARATOR_LIST);
    node->left = left;
    while (accept(COMMA)) {
        node->right = init_declarator_list();
    }

    return node;
}

ast_node_t *declaration() {
    ast_node_t *d = new_node(DECLARATION);

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
    ast_node_t *thisNode = new_node(TRANSLATION_UNIT);
    thisNode->left = external_declaration();

    while (!is_eof()) {
        thisNode->right = translation_unit();
    }
    
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
