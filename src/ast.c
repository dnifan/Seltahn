#include <stdint.h>

#include "tokens.h"
#include "ast.h"
#include "list.h"
#include "stab.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

token_t *current;
uint32_t total_tokens, token_index;
token_t **tokens;
stab_t *stab_root;
stab_t *stab_current;
linked_list *contexts;

ast_node_t *pointer();
ast_node_t *statement();
ast_node_t *declarator();
ast_node_t *expression();
ast_node_t *initializer();
ast_node_t *enum_specifier();
ast_node_t *cast_expression();
ast_node_t *postfix_expression();
ast_node_t *compound_statement();
ast_node_t *expression_statement();
ast_node_t *assignment_expression();
ast_node_t *conditional_expression();
ast_node_t *struct_or_union_specifier();
void ast_dump_start(ast_node_t *root);

token_t *ast_current() {
    return current;
}

void push() {
	list_add(contexts, (void*)(token_index-1));
}

void pop() {
	list_node *node = list_last(contexts);
	list_remove_node(contexts, node);
	
	token_index = (uint32_t)node->element;
	current = tokens[token_index];

	free(node);
}

ast_node_t *new_node(ast_node_type type) {
    ast_node_t *node = (ast_node_t*)malloc(sizeof(ast_node_t));
    memset(node, 0, sizeof(ast_node_t));
    node->type = type;
    node->token = current;
    return node;
}

uint8_t is_eof() {
    return token_index >= total_tokens;
}

void next() {
    if (is_eof())
        ast_fatal(current, "unexpected end of stream");
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
        ast_fatal(current, "Expected %s.", token_get_name_by_type(tok));

    if (!is_eof())
        next();
}

ast_node_t *identifier() {
    if (current->type == IDENTIFIER) {
        ast_node_t *result = new_node(ID);
        // TODO: add pointer to symbol table here.
        next();
        return result;
    }

    return NULL;
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
    else {
        ast_node_t *es = enum_specifier();
        if (es != NULL)
            return es;

        ast_node_t *su = struct_or_union_specifier();
        if (su != NULL)
            return su;
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

ast_node_t *parameter_declaration() {
    ast_node_t *ds = declaration_specifiers();
    if (ds == NULL)
        ast_fatal(current, "expected declaration_specifiers");

    ast_node_t *d = declarator();
    if (d == NULL)  // TODO: abstract declarator.
        ast_fatal(current, "expected declarator");

    ast_node_t *result = new_node(PARAMETER_DECLARATION);
    result->left = ds;
    result->right = d;
    return result;
}

ast_node_t *parameter_list() {
    ast_node_t *pd = parameter_declaration();
    if (pd == NULL)
        return NULL;

    ast_node_t *result = new_node(PARAMETER_LIST);
    result->left = pd;
    if (accept(COMMA)) {
        if (accept(ELLIPS))
            ast_fatal(current, "ellipsis not supported yet");
        else 
            result->right = parameter_list();
    }
    return result;
}

ast_node_t *parameter_type_list() {
    ast_node_t *pl = parameter_list();
    if (pl == NULL)
        return NULL;
    return pl;
}

ast_node_t *dd_suffix() {
    if (accept(LBRACK)) {
        ast_node_t *res = new_node(ARRAY_DECLARE);
        res->left = constant_expression();
        expect(RBRACK);

        res->right = dd_suffix();
        return res;
    }
    else if (accept(LPAREN)) {
        if (accept(RPAREN)) {
            return new_node(EMPTY);
        }

        ast_node_t *l = parameter_type_list();
        if (l == NULL)
            ast_fatal(current, "expected RPAREN or parameter type list");
        expect(RPAREN);
        return l;
    }
    else
        return NULL;
}

ast_node_t *direct_declarator() {
    ast_node_t *res = NULL;
    if (current->type == IDENTIFIER) {
        res = identifier();
    }
    else if (current->type == LPAREN) {
        next(); // (
        res = declarator(tokens);
        expect(RPAREN);
    }

    if (res == NULL)
        return NULL;

    res->right = dd_suffix();
    return res;
}

ast_node_t *declarator() {
    ast_node_t *d = new_node(DECLARATOR);
    d->left = pointer();
    d->right = direct_declarator();
    return d;
}

ast_node_t *primary_expression() {
    if (current->type == NUMBER) {
        ast_node_t *result = new_node(CONSTANT_NUMBER);
        result->param = current->param.number;
        next();
        return result;
    }
    else if (current->type == STRING) {
        ast_node_t *result = new_node(CONSTANT_STRING);
        result->param = (uint32_t)current->param.string;
        next();
        return result;
    }
    else if (current->type == IDENTIFIER) {
        return identifier();
    }
    else if (current->type == LPAREN) {
        push(); // ambiguity with casting
        next();
        ast_node_t *result = expression();
        if (result != NULL) {
            expect(RPAREN);
            return result;
        } 
        else {
            pop();
            return NULL;
        }
        
    }
    else {
        return NULL;
    }
}

ast_node_t *argument_expression_list() {
    ast_node_t *ex = assignment_expression();
    if (ex == NULL)
        return NULL;

    ast_node_t *n = new_node(ARG_EXPRESSION_LIST);
    n->left = ex;
    if (accept(COMMA))
        n->right = argument_expression_list();
    return n;
}

ast_node_t *postfix() {
    if (accept(LBRACK)) {
        ast_node_t *ex = expression();
        if (ex == NULL)
            ast_fatal(current, "expected expression");
        expect(RBRACK);

        ast_node_t *n = new_node(ARRAY_INDEX);
        n->left = ex;
        n->right = postfix();
        return n;
    }
    else if (accept(LPAREN)) {
        if (accept(RPAREN)) {
            return new_node(FUNCTION_CALL);
        }
        else {
            ast_node_t *fc = new_node(FUNCTION_CALL);
            fc->left = argument_expression_list();
            if (fc->left == NULL)
                ast_fatal(current, "expected argument expression list");
            expect(RPAREN);
            return fc;
        }
    }
    else if (accept(PERIOD)) {
        ast_node_t *id = identifier();
        if (id == NULL)
            ast_fatal(current, "identifier expected");

        ast_node_t *result = new_node(FIELD_FOLLOW);
        result->left = id;
        result->right = postfix();
        return result;
    }
    else if (accept(OP_PTR)) {
        ast_node_t *id = identifier();
        if (id == NULL)
            ast_fatal(current, "identifier expected");

        // TODO: check for crazy inverted tree output
        ast_node_t *result = new_node(PTR_FOLLOW);
        result->left = id;
        result->right = postfix();
        return result;
    }
    else if (accept(OP_INC)) {
        ast_node_t *n = new_node(INCREMENT);
        n->right = postfix();
        return n;
    }
    else if (accept(OP_DEC)) {
        ast_node_t *n = new_node(DECREMENT);
        n->right = postfix();
        return n;
    }
    else
        return NULL;
}

ast_node_t *postfix_expression() {

    ast_node_t *ex = primary_expression();
    if (ex == NULL) {
        return NULL;
    }

    ex->postfix = postfix();
    return ex;
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
        next();
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
            ast_fatal(current, "sizeof(type_name) not supported yet.");
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
    ast_node_t *ue = unary_expression();
    if (ue != NULL)
        return ue;

    if (accept(LPAREN)) {
        ast_fatal(current, "casting not supported yet");

        /*ast_node_t *tp = new_node(CAST);
        tp->left = type_name();
        if (tp->left == NULL)
            ast_fatal(current, "expected identifier");
        expect(RPAREN);
        tp->right = cast_expression();
        return tp;*/
    }

    return NULL;
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

    ast_node_t *left = conditional_expression();

    ast_node_t *ao = assignment_operator();
    if (ao == NULL) {
        return left;
    }

    ast_node_t *ae = assignment_expression();
    if (ao == NULL) {
        ast_fatal(current, "expected assignment_expression");
    }

    ast_node_t *n = new_node(ASSIGNMENT);
    n->left = left;
    n->middle = ao;
    n->right = ae;
    return n;
}

ast_node_t *expression() {
    ast_node_t *n = assignment_expression();
    if (accept(COMMA)) {
        n->right = expression();
    }
    return n;
}

ast_node_t *initializer_list() {
    ast_node_t *i = initializer();
    if (i == NULL)
        return NULL;

    ast_node_t *result = new_node(INITIALIZER_LIST);
    result->left = i;
    if (accept(COMMA))
        result->right = initializer_list();
    return result;
}

ast_node_t *initializer() {
    ast_node_t *a = assignment_expression();
    if (a != NULL)
        return a;

    if (accept(LBRACE)) {
        ast_node_t *il = initializer_list();
        accept(COMMA);
        expect(RBRACE);
        return il;
    }
    else
        return NULL;
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

ast_node_t *declaration_list() {
    ast_node_t *d = declaration();
    if (d == NULL)
        return NULL;

    ast_node_t *l = new_node(DECLARATION_LIST);
    l->left = d;
    l->right = declaration_list();
    return l;
}

ast_node_t *selection_statement() {
	if (accept(IF)) {
		ast_node_t *result = new_node(IF_STATEMENT);

		expect(LPAREN);
		result->left = expression();
		if (result->left == NULL)
			ast_fatal(current, "expected expression");
		expect(RPAREN);
		result->middle = statement();
		if (result->middle == NULL)
			ast_fatal(current, "expected statement");

		if (accept(ELSE)) {
			result->right = statement();
			if (result->right == NULL)
				ast_fatal(current, "expected statement");
		}

		return result;
	}
	else if (accept(SWITCH)) {
		ast_node_t *result = new_node(SWITCH_STATEMENT);

		expect(LPAREN);
		result->left = expression();
		if (result->left == NULL)
			ast_fatal(current, "expected expression");
		expect(RPAREN);

		result->right = statement();
		if (result->right == NULL)
			ast_fatal(current, "expected statement");

		return result;
	}
	else
		return NULL;
}

ast_node_t *labeled_statement() {
	if (accept(CASE)) {
		ast_node_t *result = new_node(CASE_STATEMENT);
		result->left = constant_expression();
		if (result->left == NULL)
			ast_fatal(current, "expected constant_expression");
		expect(COLON);
		
		result->right = statement();
		if (result->right == NULL)
			ast_fatal(current, "expected statement");

		return result;
	}
	else if (accept(DEFAULT)) {
		ast_node_t *result = new_node(DEFAULT_STATEMENT);
		expect(COLON);

		result->left = statement();
		if (result->left == NULL)
			ast_fatal(current, "expected statement");

		return result;
	}
	else
		return NULL;
}

ast_node_t *iteration_statement() {
	if (accept(WHILE)) {
		ast_node_t *result = new_node(WHILE_STATEMENT);
		
		expect(LPAREN);
		result->left = expression();
		if (result->left == NULL)
			ast_fatal(current, "expected expression");
		expect(RPAREN);

		result->right = statement();
		if (result->right == NULL)
			ast_fatal(current, "expected statement");

		return result;
	}
	else if (accept(DO)) {
		ast_node_t *result = new_node(DO_STATEMENT);

		result->left = statement();
		if (result->left == NULL)
			ast_fatal(current, "expected statement");

		expect(WHILE);
		expect(LPAREN);
		result->right = expression();
		if (result->right == NULL)
			ast_fatal(current, "expected expression");
		expect(RPAREN);
		expect(SEMICOLON);

		return result;
	}
	else if (accept(FOR)) {
        ast_node_t *result = new_node(FOR_STATEMENT);
        result->left = new_node(FOR_STMT_CONDITIONAL);

        expect(LPAREN);
        result->left->left = expression_statement();
        if (result->left->left == NULL)
            ast_fatal(current, "expected expression_statement");
        result->left->middle = expression_statement();
        if (result->left->middle == NULL)
            ast_fatal(current, "expected expression_statement");

        result->left->right = expression();
        expect(RPAREN);

        result->right = statement();
        if (result->right == NULL)
            ast_fatal(current, "expected statement");
        return result;
	}
	else
		return NULL;
}

ast_node_t *jump_statement() {
    if (accept(CONTINUE)) {
        ast_node_t *n = new_node(CONTINUE_STATEMENT);
        expect(SEMICOLON);
        return n;
    }
    else if (accept(BREAK)) {
        ast_node_t *n = new_node(BREAK_STATEMENT);
        expect(SEMICOLON);
        return n;
    }
    else if (accept(RETURN)) {
        ast_node_t *n = new_node(RETURN_STATEMENT);
        n->left = expression();
        expect(SEMICOLON);
        return n;
    }
    else
        return NULL;
}

ast_node_t *expression_statement() {
    if (accept(SEMICOLON))
        return new_node(EMPTY);
    
    ast_node_t *ex = expression();
    if (ex != NULL) {
        expect(SEMICOLON);
        return ex;
    }

    return NULL;
}

ast_node_t *statement() {
	ast_node_t *s = labeled_statement();
	if (s != NULL) 
		return s;

	s = compound_statement();
	if (s != NULL)
		return s;

    s = expression_statement();
    if (s != NULL)
        return s;

	s = selection_statement();
	if (s != NULL)
		return s;

	s = iteration_statement();
	if (s != NULL)
		return s;

    s = jump_statement();
    if (s != NULL)
        return s;

	return NULL;
}

ast_node_t *statement_list() {
	ast_node_t *s = statement();
	if (s != NULL) {
		ast_node_t *result = new_node(STATEMENT_LIST);
		result->left = s;
		result->right = statement_list();
		return result;
	}
	return NULL;
}

ast_node_t *type_qualifier_list() {
    ast_node_t *tq = type_qualifier();
    if (tq == NULL)
        return NULL;

    ast_node_t *result = new_node(TYPE_QUALIFIER_LIST);
    result->left = tq;
    result->right = type_qualifier_list();
    return result;
}

ast_node_t *enumerator() {
    ast_node_t *id = identifier();
    if (id == NULL)
        return NULL;

    ast_node_t *result = new_node(ENUMERATOR);
    result->left = id;
    if (accept(ASSIGN)) {
        result->right = constant_expression();
        if (result->right == NULL)
            ast_fatal(current, "expected constant_expression");
    }
    return result;
}

ast_node_t *struct_or_union() {
    if (accept(STRUCT)) {
        ast_node_t *r = new_node(STRUCT_OR_UNION);
        r->param = STRUCT;
        return r;
    }
    else if (accept(UNION)) {
        ast_node_t *r = new_node(STRUCT_OR_UNION);
        r->param = UNION;
        return r;
    }
    else
        return NULL;
}

ast_node_t *struct_declarator() {
    ast_node_t *result = new_node(STRUCT_DECLARATOR);
    result->left = declarator();
    
    if (accept(COLON)) {
        result->right = constant_expression();
    }

    return result;
}

ast_node_t *struct_declarator_list() {
    ast_node_t *sd = struct_declarator();
    if (sd == NULL)
        return NULL;
    
    ast_node_t *result = new_node(STRUCT_DECLARATOR_LIST);
    result->left = sd;
    if (accept(COMMA))
        result->right = struct_declarator_list();
    return result;
}

ast_node_t *specifier_qualifier_list() {
    ast_node_t *ts = type_specifier();
    if (ts != NULL) {
        ast_node_t *res = new_node(SPECIFIER_QUALIFIER_LIST);
        res->left = ts;
        res->right = specifier_qualifier_list();
        return res;
    }
    else {
        ast_node_t *tq = type_qualifier();
        if (tq != NULL) {
            ast_node_t *res = new_node(SPECIFIER_QUALIFIER_LIST);
            res->left = tq;
            res->right = specifier_qualifier_list();
            return res;
        }
        else
            return NULL;
    }
}

ast_node_t *struct_declaration() {
    ast_node_t *result = new_node(STRUCT_DECLARATION);
    result->left = specifier_qualifier_list();
    if (result->left == NULL)
        return NULL;
    result->right = struct_declarator_list();
    if (result->right == NULL)
        ast_fatal(current, "expected struct_declarator_list");
    expect(SEMICOLON);
    return result;
}

ast_node_t *struct_declaration_list() {
    ast_node_t *sd = struct_declaration();
    if (sd == NULL)
        return NULL;

    ast_node_t *result = new_node(STRUCT_DECLARATION_LIST);
    result->left = sd;
    result->right = struct_declaration_list();
    return result;
}

ast_node_t *struct_or_union_specifier() {
    ast_node_t *su = struct_or_union();
    if (su == NULL)
        return NULL;

    ast_node_t *result = new_node(STRUCT_OR_UNION_SPECIFIER);
    result->left = identifier();
    if (accept(LBRACE)) {
        result->right = struct_declaration_list();
        if (result->right == NULL)
            ast_fatal(current, "expected struct_declaration_list");
        expect(RBRACE);
    }

    return result;
}

ast_node_t *enumerator_list() {
    ast_node_t *e = enumerator();
    if (e == NULL)
        return NULL;

    ast_node_t *result = new_node(ENUMERATOR_LIST);
    result->left = e;
    if (accept(COMMA))
        result->right = enumerator_list();
    return result;
}

ast_node_t *enum_specifier() {
    if (accept(ENUM)) {
        ast_node_t *result = new_node(ENUM_SPECIFIER);
        result->left = identifier();

        if (accept(LBRACE)) {
            result->right = enumerator_list();
            if (result->right == NULL)
                ast_fatal(current, "expected enumerator_list");
            expect(RBRACE);
        }

        return result;
    }
    else
        return NULL;
}

ast_node_t *pointer() {
    if (accept(ASTERISK)) {
        ast_node_t *result = new_node(POINTER);
        result->left = type_qualifier_list();
        result->right = pointer();
        return result;
    }
    else {
        return NULL;
    }
}

ast_node_t *compound_statement() {
	if (accept(LBRACE)) {
		ast_node_t *result = new_node(COMPOUND_STATEMENT);
		result->right = statement_list();
        if (result->right == NULL) {
            result->left = declaration_list();
            result->right = statement_list();
        }
        
		expect(RBRACE);
		return result;
	}
	else
		return NULL;
}

ast_node_t *function_definition() {
	push();
	ast_node_t *ds = declaration_specifiers();
	ast_node_t *d = declarator();
	if (d != NULL) {
		ast_node_t *c = compound_statement();
		if (c != NULL) {
			ast_node_t *n = new_node(FUNCTION_DEFINITION);
			n->left = ds;
			n->middle = d;
			n->right = c;
			return n;
		}
	}
	pop();
	return NULL;
}

ast_node_t *external_declaration() {
    ast_node_t *d = function_definition();
    if (d == NULL) {
        // function declaration
		d = declaration();
		if (d == NULL) {
			ast_fatal(current, "expected function definition or declaration");
		}
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
    ast_t *ast = (ast_t*)malloc(sizeof(ast_t));
    tokens = t;
    token_index = 0;
    total_tokens = token_count;
	contexts = list_new();

    // create global scope symbol table
    stab_root = stab_new(NULL);
    stab_current = stab_root;

    next();
    ast->root_node = translation_unit();
    
	//list_free(contexts);
    return ast;
}
