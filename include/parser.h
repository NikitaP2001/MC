#include <list.h>

enum parser_symbol_type {
        psym_invalid = 0,

        psym_constant,
        psym_integer_constant,
        psym_floating_constant,
        psym_enumeration_constant,
        psym_character_constant,

        psym_expression,
        psym_postfix_expression,

        psym_last,

        psym_endmarker,
        psym_epsilon,
        psym_terminal,
};

#define PARSER_EPSILON_VALUE "e"
#define PARSER_ENDMARKER_VALUE "$"

/* is production or terminal */
struct parser_symbol {
        enum parser_symbol_type type;
        char *value;
};

#define PARSER_PROD_LEN_LIMIT 4

struct parser_production {
        enum parser_symbol_type source_type;
        struct parser_symbol derivation[PARSER_PROD_LEN_LIMIT];
};


struct pasrser_production_list {
        struct list_head link;
        struct parser_production *production;
};


/* this should be real set, but for now it`s list */
struct parser_symbol_set {
        struct list_head *link;
        struct parser_symbol *sym;
};


struct parser {

        struct parser_production_list *grammar[psym_last];
        
        struct parser_symbol_set *first_set[psym_last];

        struct parser_symbol_set *follow_set[psym_last];

};

void parser_init(struct parser *ps, enum parser_symbol_type start_sym);