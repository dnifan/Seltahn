#include <stdio.h>
#include "tokens.h"
#include "lexer.h"
#include "ast.h"

const char *example = "int main()\
{\
    char  line[100];\
    int   total;\
    int   item;\
\
    total = 0;\
    while (1) {\
        printf(\"Enter # to add \\n\");\
        printf(\"  or 0 to stop:\");\
\
        fgets(line, sizeof(line), stdin);\
        sscanf(line, \"%d\", &item);\
\
        if (item == 0)\
            break;\
\
        total += item;\
        printf(\"Total: %d\\n\", total);\
    }\
    printf(\"Final total %d\\n\", total);\
    return (0);\
}";

void tokens_debug(token_t **tokens, int token_count) {
    for (int i = 0;i < token_count;i++) {
        token_t *token = tokens[i];

        if (token->type == WHITESPACE)
            continue;

        printf("%10s (%02X) %08X", token_get_name(*token), token->type, token->param);

        if (token->type == SYMBOL || token->type == STRING) {
            printf(" (%s)", token->param.string);
        }

        printf("\n");
    }
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Please provide a file name.\n");
		return -1;
	}

	char *filename = argv[1];
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf("Unable to open file %s\n", argv[1]);
		return -2;
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	printf("File size: %i bytes\n", size);
	char *buffer = (char*)malloc(size+1);
	memset(buffer, 0, size + 1);
	fread(buffer, 1, size, fp);
	fclose(fp);

    uint32_t token_count;
    //lexer_state *state = lex_init("unsigned static int a() { if (1 == 1 && 500 >= (6<<3)) { switch(3-1==0) { case 1: {} default: { } } } else { } }");
	//lexer_state *state = lex_init("int my_func() { do { } while (1<2); }");
	lexer_state *state = lex_init(buffer);
    token_t **tokens = lex_run(state, &token_count);
    ast_t *ast = ast_create(tokens, token_count);
    
    lex_destroy(state);
    return 0;
}
