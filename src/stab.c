#include <stdint.h>

#include "list.h"
#include "stab.h"
#include "tokens.h"
#include "ast.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

stab_t *stab_new(stab_t *prev) {
    stab_t *stab = (stab_t*)malloc(sizeof(stab_t));
    memset(stab, 0, sizeof(stab));

    stab->list = list_new();
    stab->previous = prev;

    return stab;
}

symbol_t *symbol_new(const char *name, uint32_t size) {
    symbol_t *smb = (symbol_t*)malloc(sizeof(symbol_t));
    smb->symbol = (const char *)malloc(strlen(name));
    strcpy((char *)smb->symbol, name);
    smb->size = size;
    return smb;
}

void stab_add(stab_t *stab, symbol_t *record) {
    if (stab_get(stab, record->symbol) != NULL) {
        ast_fatal(ast_current(), "Symbol '%s' has already been declared", record->symbol);
    }
    
    list_add(stab->list, record);
    printf("Symbol '%s' declared!\n", record->symbol);
}

symbol_t *stab_get(stab_t *stab, const char *name) {
    // Walk through the linked symbol tables.
    for (stab_t *table = stab; table != NULL; table = table->previous) {
        // Walk through the linked list to find the symbol with this name.
        for (uint32_t i = 0; i < table->list->length; i++) {
            symbol_t *smb = (symbol_t*)list_at(table->list, i);
            if (strcmp(smb->symbol, name) == 0) {
                return smb;
            }
        }
    }

    return NULL;
}
