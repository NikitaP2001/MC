#ifndef _PARSER_H_
#define _PARSER_H_
#include <stdbool.h>
#include <list.h>
#include <token.h>
#include <mc.h>

extern const char *parser_symbol_nonterms[];

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
        psym_type_qualifier_list2,
        psym_struct_declaration,
        psym_struct_declarator_list,
        psym_struct_declarator_list2,
        psym_struct_declarator,
        psym_translation_unit,
        psym_translation_unit2,
        psym_external_declaration,
        psym_parameter_list,
        psym_parameter_list2,
        psym_parameter_declaration,
        psym_declaration_specifiers,
        psym_declarator,
        psym_storage_class_specifier,
        psym_function_specifier,
        psym_function_definition,
        psym_declaration,
        psym_direct_declarator,
        psym_direct_declarator2,
        psym_declaration_list,
        psym_declaration_list2,
        psym_compound_statement,
        psym_init_declarator_list,
        psym_init_declarator_list2,
        psym_block_item_list,
        psym_block_item_list2,
        psym_init_declarator,
        psym_block_item,
        psym_statement,
        psym_labeled_statement,
        psym_expression_statement,
        psym_selection_statement,
        psym_iteration_statement,
        psym_jump_statement,
        psym_identifier_list,
        psym_identifier_list2,

        psym_first_nonterm = psym_expression,
        psym_last_nonterm = psym_identifier_list2,

        psym_identifier,
        psym_constant,
        psym_string_literal,

        psym_first_punc,
        /* enum punc_type lays here */
        psym_last_punc = psym_first_punc + punc_last,

        psym_first_keyw,
        /* enum keyword_type lays here */
        psym_last_keyw = psym_first_keyw + keyw_last,

        psym_endmarker,

        psym_first_term = psym_identifier,
        psym_last_term = psym_endmarker,

        psym_epsilon,

        psym_last = psym_epsilon,
};

/* calculates psym enum code based on punctuator token code */
#define PARSER_PUNCTUATOR(tok_code) psym_first_punc + tok_code
/* calculates psym enum code based on keyword token code */
#define PARSER_KEYWORD(tok_code) psym_first_keyw + tok_code

#define PARSER_NUM_TERM  (psym_last_term - psym_first_term + 1)
#define PARSER_NUM_NONTERM (psym_last_nonterm - psym_first_nonterm + 1)
#define PARSER_NUM_SYM   (psym_last + 1)

static inline _Bool
parser_symbol_is_terminal(enum parser_symbol sym)
{
        return (sym >= psym_first_term && sym <= psym_last_term);
}

static inline _Bool 
parser_symbol_not_terminal(enum parser_symbol sym)
{
        return (sym >= psym_first_nonterm && sym <= psym_last_nonterm);
}

static inline 
const char *
parser_symbol_to_str(enum parser_symbol sym)
{
        if (sym >= psym_first_punc && sym <= psym_last_punc)
                return token_punc_to_str((enum punc_type)sym);
        else if (sym >= psym_first_keyw && sym <= psym_last_keyw)
                return token_keyw_to_str((enum keyword_type)sym);
        else if (parser_symbol_not_terminal((enum parser_symbol)sym))
                return parser_symbol_nonterms[sym - psym_first_nonterm];
        else switch (sym)  {
                case psym_identifier:
                        return "identifier";
                case psym_constant:
                        return "constant";
                case psym_string_literal:
                        return "string_literal";
                case psym_endmarker:
                        return "endmarker";
                case psym_epsilon:
                        return "epsilon";
                default:
                        return "Invalid symbol value";
        }
}

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
typedef enum mc_status (*error_callback)(void *pp_data, enum parser_symbol top);
typedef void (*output_production)(void *pp_data, struct parser_production prod);

struct parser_ops {
        next_symbol_callback next_sym;
        error_callback error;
        output_production output;
};

struct parser {

        struct parser_ops ops; 

        struct parser_production_list *grammar[PARSER_NUM_NONTERM];
        struct parser_symbol_set first_set[PARSER_NUM_NONTERM];
        struct parser_symbol_set follow_set[PARSER_NUM_NONTERM];
        struct parser_production table[PARSER_NUM_NONTERM][PARSER_NUM_TERM];
        enum parser_symbol start_sym;
};

void parser_init(struct parser *ps, struct parser_ops ops, 
        enum parser_symbol start_sym);

void parser_free(struct parser *ps);

enum mc_status parser_process(struct parser *ps, void *pp_data);

#endif /* _PARSER_H_ */