#ifndef _SYMBOL_H_
#define _SYMBOL_H_
#include <token.h>

enum parser_symbol {
        psym_invalid = 0,

        psym_translation_unit,

        psym_expression,
        psym_conditional_expression,
        psym_logical_or_expression,
        psym_logical_and_expression,
        psym_inclusive_or_expression,
        psym_exclusive_or_expression,
        psym_and_expression,
        psym_equality_expression,
        psym_relational_expression,
        psym_shift_expression,
        psym_additive_expression,
        psym_multiplicative_expression,
        psym_cast_expression,
        psym_unary_expression,
        psym_type_name,
        psym_postfix_expression,
        psym_unary_operator,
        psym_primary_expression,
        psym_initializer_list,
        psym_argument_expression_list,
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
        psym_enumerator,
        psym_enumeration_constant,
        psym_designation,
        psym_initializer,
        psym_designator_list,
        psym_designator,
        psym_pointer,
        psym_direct_abstract_declarator,
        psym_parameter_type_list,
        psym_struct_or_union,
        psym_struct_declaration_list,
        psym_type_qualifier_list,
        psym_struct_declaration,
        psym_struct_declarator_list,
        psym_struct_declarator,
        psym_external_declaration,
        psym_parameter_list,
        psym_parameter_declaration,
        psym_declaration_specifiers,
        psym_declarator,
        psym_storage_class_specifier,
        psym_function_specifier,
        psym_function_definition,
        psym_declaration,
        psym_direct_declarator,
        psym_declaration_list,
        psym_compound_statement,
        psym_init_declarator_list,
        psym_block_item_list,
        psym_init_declarator,
        psym_block_item,
        psym_statement,
        psym_labeled_statement,
        psym_expression_statement,
        psym_selection_statement,
        psym_iteration_statement,
        psym_jump_statement,
        psym_identifier_list,

        psym_first_nonterm = psym_translation_unit,
        psym_last_nonterm = psym_identifier_list,

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

        psym_last = psym_last_term,
};

#define PARSER_KEYWORD(type) (type + psym_first_keyw)
#define PARSER_PUNCTUATOR(type) (type + psym_first_punc)

#endif /* _SYMBOL_H_ */