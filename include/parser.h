#include <stdbool.h>
#include <list.h>

#define PARSER_SYM_INVALID -1

enum parser_symbol {
        psym_invalid = 0,

        psym_constant,
        psym_integer_constant,
        psym_floating_constant,
        psym_enumeration_constant,
        psym_character_constant,
        psym_expression,
        psym_postfix_expression,

        psym_last_nonterm = psym_postfix_expression,

        psym_left_square_bracket,
        psym_right_square_bracket,

        psym_last_term = psym_right_square_bracket,

        psym_endmarker,
        psym_epsilon,

        psym_last = psym_epsilon,
};

#define PARSER_FIRST_NONTERM psym_constant
#define PARSER_FIRST_TERM psym_left_square_bracket

#define PARSER_NUM_TERM  (psym_last_term - PARSER_FIRST_TERM + 1)
#define PARSER_NUM_NONTERM (psym_last_nonterm - PARSER_FIRST_NONTERM + 1)
#define PARSER_NUM_SYM   (psym_last - PARSER_FIRST_TERM)

#define PARSER_PROD_LEN_LIMIT 10

struct parser_production {
        enum parser_symbol source;
        size_t n_deriv;
        enum parser_symbol derivation[PARSER_PROD_LEN_LIMIT];
};


struct parser_production_list {
        struct list_head link;
        struct parser_production *production;
};

struct parser_symbol_set {
        int n_elem;
        _Bool syms[PARSER_NUM_SYM];
};

struct parser {

        struct parser_production_list *grammar[PARSER_NUM_NONTERM];
        
        struct parser_symbol_set first_set[PARSER_NUM_NONTERM];

        struct parser_symbol_set follow_set[PARSER_NUM_NONTERM];

        struct parser_production table[PARSER_NUM_NONTERM][PARSER_NUM_SYM];
};

void parser_init(struct parser *ps, enum parser_symbol start_sym);

void parser_free(struct parser *ps);