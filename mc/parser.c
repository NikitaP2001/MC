#include <stdint.h>

enum parser_symbol_type {

        psym_constant,
        psym_integer_constant,
        psym_floating_constant,
        psym_enumeration_constant,
        psym_character_constant,

        psym_expression,
        psym_postfix_expression,

        psym_identifier,
        psym_terminal,
};

/* is production or terminal */
struct parser_grammar_symbol {
        enum parser_symbol_type type;
        char *value;
};

#define PARSER_GRAMMAR_MAX_PROD_LEN 4

struct parser_grammar_production {
        enum parser_symbol_type source_type;
        struct parser_grammar_symbol derivation[PARSER_GRAMMAR_MAX_PROD_LEN];
};

static const struct parser_grammar_production parser_grammar[] = {
        { psym_postfix_expression, {    { psym_postfix_expression, NULL }, 
                                        { psym_terminal, "["}, 
                                        { psym_expression, NULL },
                                        { psym_terminal, "]"}, }
        },

};

struct parser {
        int fuck_you;
        

};

static void 
parser_first_set(struct parser *ps, enum parser_symbol_type type)
{

}