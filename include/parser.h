#include <list.h>

enum parser_symbol {
        psym_constant,
        psym_integer_constant,
        psym_floating_constant,
        psym_enumeration_constant,
        psym_character_constant,

        psym_expression,
        psym_postfix_expression,

        psym_last_nterm,

        psym_left_square_bracket,
        

        psym_last_term,

        psym_endmarker,
        psym_epsilon,
};

#define PARSER_NUM_TERM  (psym_last_term - psym_last_nterm - 1)
#define PARSER_NUM_NTERM (psym_last_nterm)
#define PARSER_NUM_SYM   (psym_epsilon)

#define PARSER_PROD_LEN_LIMIT 4

struct parser_production {
        enum parser_symbol source;
        size_t n_deriv;
        enum parser_symbol derivation[PARSER_PROD_LEN_LIMIT];
};


struct parser_production_list {
        struct list_head link;
        struct parser_production *production;
};


/* this should be real set, but for now it`s list */
struct parser_symbol_set {
        size_t n_elem;
        _Bool syms[PARSER_NUM_SYM];
};

#define PARSER_TERM_SYM_COUNT 32

struct parser_table_row {
        struct parser_production *entry[PARSER_NUM_TERM];
};

struct parser {

        struct parser_production_list *grammar[PARSER_NUM_NTERM];
        
        struct parser_symbol_set first_set[PARSER_NUM_NTERM];

        struct parser_symbol_set follow_set[PARSER_NUM_NTERM];

        struct parser_table_row *table[PARSER_NUM_NTERM];
};

void parser_init(struct parser *ps, enum parser_symbol start_sym);

void parser_free(struct parser *ps);