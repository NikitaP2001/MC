#ifndef _SYMSET_H_
#define _SYMSET_H_
#include <parser.h>

/* The purpose of this module is to help us identify
 * the type of production ahead, by looking on first of more symbols
 */

#define STRUCT_OR_UNION                         \
        [PARSER_KEYWORD(keyw_struct)] = 1,      \
        [PARSER_KEYWORD(keyw_union)] = 1,       \

#define STRUCT_OR_UNION_SPECIFIER               \
        STRUCT_OR_UNION

#define ENUM_SPECIFIER                          \
        [PARSER_KEYWORD(keyw_enum)] = 1,

#define TYPEDEF_NAME                            \
        [psym_identifier] = 1,

#define TYPE_SPECIFIER                          \
        [PARSER_KEYWORD(keyw_void)] = 1,        \
        [PARSER_KEYWORD(keyw_char)] = 1,        \
        [PARSER_KEYWORD(keyw_short)] = 1,       \
        [PARSER_KEYWORD(keyw_int)] = 1,         \
        [PARSER_KEYWORD(keyw_long)] = 1,        \
        [PARSER_KEYWORD(keyw_float)] = 1,       \
        [PARSER_KEYWORD(keyw_double)] = 1,      \
        [PARSER_KEYWORD(keyw_signed)] = 1,      \
        [PARSER_KEYWORD(keyw_unsigned)] = 1,    \
        [PARSER_KEYWORD(keyw_Bool)] = 1,        \
        [PARSER_KEYWORD(keyw_Complex)] = 1,     \
        [PARSER_KEYWORD(keyw_Imaginary)] = 1,   \
        STRUCT_OR_UNION_SPECIFIER               \
        ENUM_SPECIFIER                          \
        TYPEDEF_NAME

#define TYPE_QUALIFIER                          \
        [PARSER_KEYWORD(keyw_const)] = 1,       \
        [PARSER_KEYWORD(keyw_restrict)] = 1,    \
        [PARSER_KEYWORD(keyw_volatile)] = 1,

#define STORAGE_CLASS_SPECIFIER                 \
        [PARSER_KEYWORD(keyw_typedef)] = 1,       \
        [PARSER_KEYWORD(keyw_extern)] = 1,       \
        [PARSER_KEYWORD(keyw_static)] = 1,       \
        [PARSER_KEYWORD(keyw_auto)] = 1,       \
        [PARSER_KEYWORD(keyw_register)] = 1,

#define FUNCTION_SPECIFIER                      \
        [PARSER_KEYWORD(keyw_inline)] = 1,

#define POINTER                                 \
        [PARSER_PUNCTUATOR(punc_mul)] = 1,

#define DIRECT_DECLARATOR                               \
        [psym_identifier] = 1,                          \
        [PARSER_PUNCTUATOR(punc_left_rnd_br)] = 1,

#define DECLARATOR                              \
        POINTER                                 \
        DIRECT_DECLARATOR                       \

#define DECLARATION_SPECIFIERS                  \
        STORAGE_CLASS_SPECIFIER                 \
        TYPE_SPECIFIER                          \
        TYPE_QUALIFIER                          \
        FUNCTION_SPECIFIER

#define DECLARATION                             \
        DECLARATION_SPECIFIERS

#define TYPE_NAME                               \
        TYPE_SPECIFIER                          \
        TYPE_QUALIFIER

#define DECLARATION_LIST                        \
        DECLARATION_SPECIFIERS

#define STRUCT_DECLARATION                      \
        TYPE_SPECIFIER                          \
        TYPE_QUALIFIER

/* first for  struct decl list */
#define STRUCT_DECLARATION_LIST                 \
        STRUCT_DECLARATION

#define DECLARATION_SECOND                              \
        [PARSER_PUNCTUATOR(punc_semicolon)] = 1,        \
        [PARSER_PUNCTUATOR(punc_assign)] = 1,           \
        [PARSER_PUNCTUATOR(punc_comma)] = 1,

#define TRANSLATION_UNIT                        \
        DECLARATION_SPECIFIERS

#define STRUCT_DECLARATOR                       \
        DECLARATOR                              \
        [PARSER_PUNCTUATOR(punc_colon)] = 1,

#define PRIMARY_EXPRESSION                              \
        [psym_identifier] = 1,                          \
        [psym_constant] = 1,                            \
        [psym_string_literal] = 1,                      \
        [PARSER_PUNCTUATOR(punc_comma)] = 1,

#define POSTFIX_EXPRESSION                              \
        PRIMARY_EXPRESSION

#define POSTFIX_EXPRESSION_SECOND                       \
        [PARSER_PUNCTUATOR(punc_left_sq_br)] = 1,       \
        [PARSER_PUNCTUATOR(punc_left_rnd_br)] = 1,      \
        [PARSER_PUNCTUATOR(punc_dot)] = 1,              \
        [PARSER_PUNCTUATOR(punc_right_arrow)] = 1,      \
        [PARSER_PUNCTUATOR(punc_increment)] = 1,        \
        [PARSER_PUNCTUATOR(punc_decrement)] = 1,


#define UNARY_OPERATOR                                  \
        [PARSER_PUNCTUATOR(punc_bit_and)] = 1,          \
        [PARSER_PUNCTUATOR(punc_mul)] = 1,              \
        [PARSER_PUNCTUATOR(punc_add)] = 1,              \
        [PARSER_PUNCTUATOR(punc_sub)] = 1,              \
        [PARSER_PUNCTUATOR(punc_tilde)] = 1,            \
        [PARSER_PUNCTUATOR(punc_exclamation)] = 1,

#define UNARY_EXPRESSION                                \
        POSTFIX_EXPRESSION                              \
        [PARSER_PUNCTUATOR(punc_increment)] = 1,        \
        [PARSER_PUNCTUATOR(punc_decrement)] = 1,        \
        UNARY_OPERATOR                                  \
        [PARSER_KEYWORD(keyw_sizeof)] = 1,              \

#define CAST_EXPRESSION                                 \
        UNARY_EXPRESSION

#define MULTIPLICATIVE_EXPRESSION                       \
        CAST_EXPRESSION

#define ADDITIVE_EXPRESSION                             \
        MULTIPLICATIVE_EXPRESSION

#define SHIFT_EXPRESSION                                \
        ADDITIVE_EXPRESSION

#define RELATIONAL_EXPRESSION                           \
        SHIFT_EXPRESSION

#define EQUALITY_EXPRESSION                             \
        RELATIONAL_EXPRESSION

#define AND_EXPRESSION                                  \
        EQUALITY_EXPRESSION

#define EXCLUSIVE_OR_EXPRESSION                         \
        AND_EXPRESSION

#define INCLUSIVE_OR_EXPRESSION                         \
        EXCLUSIVE_OR_EXPRESSION

#define LOGICAL_AND_EXPRESSION                          \
        INCLUSIVE_OR_EXPRESSION

#define LOGICAL_OR_EXPRESSION                           \
        LOGICAL_AND_EXPRESSION

#define CONDITIONAL_EXPRESSION                          \
        LOGICAL_OR_EXPRESSION

#define CONSTANT_EXPRESSION                             \
        CONDITIONAL_EXPRESSION

#define ASSIGNMENT_EXPRESSION                           \
        CONDITIONAL_EXPRESSION

#define EXPRESSION                                      \
        ASSIGNMENT_EXPRESSION

#define ASSIGNMENT_OPERATOR                             \
        [PARSER_PUNCTUATOR(punc_assign)] = 1,           \
        [PARSER_PUNCTUATOR(punc_mul_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_div_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_mod_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_add_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_sub_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_shl_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_shr_assigh)] = 1,       \
        [PARSER_PUNCTUATOR(punc_and_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_xor_assign)] = 1,       \
        [PARSER_PUNCTUATOR(punc_or_assign)] = 1,

#define DESIGNATOR                                      \
        [PARSER_PUNCTUATOR(punc_left_sq_br)] = 1,       \
        [PARSER_PUNCTUATOR(punc_dot)] = 1,

#define DESIGNATION_LIST                                \
        DESIGNATOR                                      

#define DESIGNATION                                     \
        DESIGNATION_LIST

#define INITIALIZER                                     \
        ASSIGNMENT_EXPRESSION                           \
        [PARSER_PUNCTUATOR(punc_left_ql_br)] = 1,

#define INITIALIZER_LIST                                \
        DESIGNATION                                     \
        INITIALIZER

#define ARGUMENT_EXPRESSION_LIST                        \
        ASSIGNMENT_EXPRESSION

#define DIRECT_ABSTRACT_DECLARATOR                      \
        [PARSER_PUNCTUATOR(punc_left_rnd_br)] = 1,      \
        [PARSER_PUNCTUATOR(punc_left_sq_br)] = 1,

#define ABSTRACT_DECLARATOR                             \
        POINTER                                         \
        DIRECT_ABSTRACT_DECLARATOR

#define ENUMERATION_CONSTANT                            \
        [psym_identifier] = 1,

#define ENUMERATOR                                      \
        ENUMERATION_CONSTANT

#define ENUMERATOR_LIST                                 \
        ENUMERATOR

#define TYPE_QUALIFIER_LIST                             \
        TYPE_QUALIFIER

#define PARAMETER_DECLARATION                           \
        DECLARATION_SPECIFIERS

#define PARAMETER_LIST                                  \
        PARAMETER_DECLARATION

#define PARAMETER_TYPE_LIST                             \
        PARAMETER_LIST

#define COMPOUND_STATEMENT                              \
        [PARSER_PUNCTUATOR(punc_left_sq_br)] = 1,

#define LABELED_STATEMENT                               \
        [psym_identifier] = 1,                          \
        [PARSER_KEYWORD(keyw_case)] = 1,                \
        [PARSER_KEYWORD(keyw_default)] = 1,

#define EXPRESSION_STATEMENT                            \
        EXPRESSION                                      \
        [PARSER_PUNCTUATOR(punc_semicolon)] = 1,

#define SELECTION_STATEMENT                             \
        [PARSER_KEYWORD(keyw_if)] = 1,                  \
        [PARSER_KEYWORD(keyw_switch)] = 1,

#define ITERATION_STATEMENT                             \
        [PARSER_KEYWORD(keyw_while)] = 1,               \
        [PARSER_KEYWORD(keyw_do)] = 1,                  \
        [PARSER_KEYWORD(keyw_for)] = 1,

#define JUMP_STATEMENT                                  \
        [PARSER_KEYWORD(keyw_goto)] = 1,                \
        [PARSER_KEYWORD(keyw_continue)] = 1,            \
        [PARSER_KEYWORD(keyw_break)] = 1,               \
        [PARSER_KEYWORD(keyw_return)] = 1,

#define STATEMENT                                       \
        /* from labeled stmt, ommited because of        \
         *overwritting identifier */                    \
        [PARSER_KEYWORD(keyw_case)] = 1,                \
        [PARSER_KEYWORD(keyw_default)] = 1,             \
        COMPOUND_STATEMENT                              \
        EXPRESSION_STATEMENT                            \
        SELECTION_STATEMENT                             \
        ITERATION_STATEMENT                             \
        JUMP_STATEMENT

#endif /* _SYMSET_H_ */