struct symbol_t {
    const char *symbol;
    uint32_t size;
};
typedef struct symbol_t symbol_t;

struct stab_t {
    linked_list *list;
    struct stab_t *previous;
};
typedef struct stab_t stab_t;

symbol_t *symbol_new(const char *name, uint32_t size);
stab_t *stab_new(stab_t *prev);
void stab_add(stab_t *stab, symbol_t *record);
symbol_t *stab_get(stab_t *stab, const char *);
