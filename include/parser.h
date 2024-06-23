#include <stdbool.h>
#include <list.h>
#include <mc.h>

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

        psym_first_nonterm = psym_constant,
        psym_last_nonterm = psym_postfix_expression,

        psym_left_square_bracket,
        psym_right_square_bracket,

        psym_first_term = psym_left_square_bracket,
        psym_last_term = psym_right_square_bracket,

        psym_endmarker,
        psym_epsilon,

        psym_last = psym_epsilon,
};

#define PARSER_NUM_TERM  (psym_last_term - psym_first_term + 1)
#define PARSER_NUM_NONTERM (psym_last_nonterm - psym_first_nonterm + 1)
#define PARSER_NUM_SYM   (psym_last)

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

typedef struct token *(*next_symbol_callback)(void *pp_data);
/* file name for last symbol read */
typedef char *(*current_file_callback)(void *pp_data);
typedef size_t *(*current_line_callback)(void *pp_data);
typedef enum mc_status (*error_callback)(void *pp_data, enum parser_symbol top);
typedef void (*output_production)(void *pp_data, struct parser_production prod);

struct parser {

        next_symbol_callback next_sym;
        current_file_callback get_curr_file;
        current_line_callback get_curr_line;
        error_callback error;
        output_production output;

        struct parser_production_list *grammar[PARSER_NUM_NONTERM];
        struct parser_symbol_set first_set[PARSER_NUM_NONTERM];
        struct parser_symbol_set follow_set[PARSER_NUM_NONTERM];
        struct parser_production table[PARSER_NUM_NONTERM][PARSER_NUM_SYM];
        enum parser_symbol start_sym;
};

void parser_init(struct parser *ps, enum parser_symbol start_sym);

void parser_free(struct parser *ps);

enum mc_status parser_process(struct parser *ps, void *pp_data);