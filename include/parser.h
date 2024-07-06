#ifndef _PARSER_H_
#define _PARSER_H_
#include <stdbool.h>
#include <list.h>
#include <token.h>
#include <mc.h>

enum parser_symbol {
        psym_invalid = 0,

        psym_expression,
        psym_expression2,
        psym_conditional_expression,
        psym_logical_or_expression,
        psym_logical_or_expression2,
        psym_logical_and_expression,
        psym_logical_and_expression2,
        psym_inclusive_or_expression,
        psym_inclusive_or_expression2,
        psym_exclusive_or_expression,
        psym_exclusive_or_expression2,
        psym_and_expression,
        psym_and_expression2,
        psym_equality_expression,
        psym_equality_expression2,
        psym_relational_expression,
        psym_relational_expression2,
        psym_shift_expression,
        psym_shift_expression2,
        psym_additive_expression,
        psym_additive_expression2,
        psym_multiplicative_expression,
        psym_multiplicative_expression2,
        psym_cast_expression,
        psym_unary_expression,
        psym_type_name,
        psym_postfix_expression,
        psym_postfix_expression2,
        psym_unary_operator,
        psym_primary_expression,
        psym_initializer_list,
        psym_initializer_list2,
        psym_argument_expression_list,
        psym_argument_expression_list2,
        psym_assignment_expression,
        psym_assignment_operator,
        psym_constant_expression,
        psym_specifier_qualifier_list,
        psym_abstract_declarator,
        psym_type_specifier,
        psym_type_qualifier,
        psym_struct_or_union_specifier,
        psym_enum_specifier,
        psym_typedef_name,
        psym_enumerator_list,
        psym_enumerator_list2,
        psym_enumerator,
        psym_enumeration_constant,
        psym_designation,
        psym_initializer,
        psym_designator_list,
        psym_designator_list2,
        psym_designator,
        psym_pointer,
        psym_direct_abstract_declarator,
        psym_direct_abstract_declarator2,
        psym_parameter_type_list,
        psym_struct_or_union,
        psym_struct_declaration_list,
        psym_struct_declaration_list2,
        psym_type_qualifier_list,
        psym_struct_declaration,
        psym_struct_declarator_list,
        psym_struct_declarator_list2,
        psym_struct_declarator,
        psym_translation_unit,
        psym_translation_unit2,
        psym_external_declaration, /* - */

        psym_first_nonterm = psym_expression,
        psym_last_nonterm = psym_pointer,

        psym_identifier,
        psym_constant,
        psym_string_literal,

        psym_first_punc,
        /* enum punc_type lays here */
        psym_last_punc = psym_first_punc + punc_last,

        psym_first_keyw,
        /* enum keyword_type lays here */
        psym_last_keyw = psym_first_keyw + keyw_last,

        psym_first_term = psym_identifier,
        psym_last_term = psym_last_keyw,

        psym_endmarker,
        psym_epsilon,

        psym_last = psym_epsilon,
};

/* calculates psym enum code based on punctuator token code */
#define PARSER_PUNCTUATOR(tok_code) psym_first_punc + tok_code
/* calculates psym enum code based on keyword token code */
#define PARSER_KEYWORD(tok_code) psym_first_keyw + tok_code

#define PARSER_NUM_TERM  (psym_last_term - psym_first_term + 1)
#define PARSER_NUM_NONTERM (psym_last_nonterm - psym_first_nonterm + 1)
#define PARSER_NUM_SYM   (psym_last)

#define PARSER_PROD_LEN_LIMIT 10

#define PARSER_PROD_DEF(...) ARRAY_SIZE(((enum parser_symbol[]) \
        { __VA_ARGS__ })), { __VA_ARGS__ }

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

#endif /* _PARSER_H_ */