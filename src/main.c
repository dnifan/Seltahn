#include "tokens.h"
#include "lexer.h"
#include "ast.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

__declspec(dllimport) unsigned long __stdcall GetTickCount();

void tokens_debug(token_t **tokens, int token_count) {
    for (int i = 0;i < token_count;i++) {
        token_t *token = tokens[i];

        if (token->type == WHITESPACE)
            continue;

        printf("%10s (%02X) %08X", token_get_name(*token), token->type, token->param.number);

        if (token->type == IDENTIFIER || token->type == STRING) {
            printf(" (%s)", token->param.string);
        }

        printf("\n");
    }
}

const char *read_file(const char *file) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
    {
        printf("Unable to open file %s\n", file);
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("File size: %i bytes\n", size);
    char *buffer = (char*)malloc(size + 1);
    memset(buffer, 0, size + 1);
    fread(buffer, 1, size, fp);
    fclose(fp);

    return buffer;
}

int main(int argc, char *argv[]) {
    uint32_t start, end, token_count;
    const char *code;
    lexer_state *state;
    token_t **tokens;
    ast_t *ast;

	if (argc < 2) {
		printf("Please provide a file name.\n");
		return -1;
	}

    code = read_file(argv[1]);
    
    start = GetTickCount();
    state = lex_init(code);
    tokens = lex_run(state, &token_count);
    ast = ast_create(tokens, token_count);
    end = GetTickCount();

    // debug information
    ast_dump_start(ast->root_node);
    printf("total compile time: %d ms\n\n", end - start);
    
    lex_destroy(state);
    return 0;
}
