#include <parser.h>

struct parser_production parser_grammar[] = {

        { psym_labeled_statement,
                PARSER_PROD_DEF(psym_identifier,
                                PARSER_PUNCTUATOR(punc_colon),
                                psym_statement)
        },
        { psym_labeled_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_case),
                                psym_constant_expression,
                                PARSER_PUNCTUATOR(punc_colon),
                                psym_statement)
        },
        { psym_labeled_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_default),
                                PARSER_PUNCTUATOR(punc_colon),
                                psym_statement)
        },

        { psym_identifier_list,
                PARSER_PROD_DEF(psym_identifier,
                                psym_identifier_list2)
        },
        { psym_identifier_list2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma), 
                                psym_identifier, 
                                psym_identifier_list2)
        },
        { psym_identifier_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },
        
        { psym_direct_declarator,
                PARSER_PROD_DEF(psym_identifier,
                                psym_direct_declarator2)
        },
        { psym_direct_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_declarator,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_type_qualifier_list,
                                psym_assignment_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_assignment_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_type_qualifier_list,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_KEYWORD(keyw_static),
                                psym_type_qualifier_list,
                                psym_assignment_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_KEYWORD(keyw_static),
                                psym_assignment_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_type_qualifier_list,
                                PARSER_KEYWORD(keyw_static),
                                psym_assignment_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_type_qualifier_list,
                                PARSER_PUNCTUATOR(punc_mul),
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_PUNCTUATOR(punc_mul),
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_parameter_type_list,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_identifier_list,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_declarator2)
        },
        { psym_direct_declarator2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_struct_declarator,
                PARSER_PROD_DEF(psym_declarator)
        },
        { psym_struct_declarator,
                PARSER_PROD_DEF(psym_declarator,
                                PARSER_PUNCTUATOR(punc_colon),
                                psym_constant_expression)
        },
        { psym_struct_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_colon),
                                psym_constant_expression)
        },

        { psym_type_qualifier_list,
                PARSER_PROD_DEF(psym_type_qualifier,
                                psym_type_qualifier_list2)
        },
        { psym_type_qualifier_list2,
                PARSER_PROD_DEF(psym_type_qualifier,
                                psym_type_qualifier_list2)
        },
        { psym_type_qualifier_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_jump_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_goto),
                                psym_identifier,
                                PARSER_PUNCTUATOR(punc_semicolon))
        },
        { psym_jump_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_continue),
                                PARSER_PUNCTUATOR(punc_semicolon))
        },
        { psym_jump_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_break),
                                PARSER_PUNCTUATOR(punc_semicolon))
        },
        { psym_jump_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_return),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon))
        },
        { psym_jump_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_return),
                                PARSER_PUNCTUATOR(punc_semicolon))
        },

        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_while),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_do),
                                psym_statement,
                                PARSER_KEYWORD(keyw_while),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                PARSER_PUNCTUATOR(punc_semicolon))
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_declaration,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_declaration,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_declaration,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_iteration_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_for),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_declaration,
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_semicolon),
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },

        { psym_selection_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_if),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },
        { psym_selection_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_if),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement,
                                PARSER_KEYWORD(keyw_else),
                                psym_statement)
        },
        { psym_selection_statement,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_switch),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_statement)
        },

        { psym_expression_statement,
                PARSER_PROD_DEF(psym_expression,
                                PARSER_PUNCTUATOR(punc_semicolon))
        },
        { psym_expression_statement,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_semicolon))
        },

        { psym_statement,
                PARSER_PROD_DEF(psym_labeled_statement)
        },
        { psym_statement,
                PARSER_PROD_DEF(psym_compound_statement)
        },
        { psym_statement,
                PARSER_PROD_DEF(psym_expression_statement)
        },
        { psym_statement,
                PARSER_PROD_DEF(psym_selection_statement)
        },
        { psym_statement,
                PARSER_PROD_DEF(psym_iteration_statement)
        },
        { psym_statement,
                PARSER_PROD_DEF(psym_jump_statement)
        },

        { psym_block_item,
                PARSER_PROD_DEF(psym_declaration)
        },
        { psym_block_item,
                PARSER_PROD_DEF(psym_statement)
        },

        { psym_block_item_list,
                PARSER_PROD_DEF(psym_block_item,
                                psym_block_item_list2)
        },
        { psym_block_item_list2,
                PARSER_PROD_DEF(psym_block_item,
                                psym_block_item_list2)
        },
        { psym_block_item_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_init_declarator,
                PARSER_PROD_DEF(psym_declarator)
        },
        { psym_init_declarator,
                PARSER_PROD_DEF(psym_declarator,
                                PARSER_PUNCTUATOR(punc_assign),
                                psym_initializer)
        },

        { psym_init_declarator_list,
                PARSER_PROD_DEF(psym_init_declarator,
                                psym_init_declarator_list2)
        },
        { psym_init_declarator_list2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_init_declarator,
                                psym_init_declarator_list2)
        },
        { psym_init_declarator_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_compound_statement,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_block_item_list,
                                PARSER_PUNCTUATOR(punc_left_ql_br))
        },
        { psym_compound_statement,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_ql_br),
                                PARSER_PUNCTUATOR(punc_left_ql_br))
        },

        { psym_declaration_list,
                PARSER_PROD_DEF(psym_declaration,
                                psym_declaration_list2)
        },
        { psym_declaration_list2,
                PARSER_PROD_DEF(psym_declaration,
                                psym_declaration_list2)
        },
        { psym_declaration_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_declaration, 
                PARSER_PROD_DEF(psym_declaration_specifiers,
                                psym_init_declarator_list,
                                PARSER_PUNCTUATOR(punc_semicolon))
        },
        { psym_declaration, 
                PARSER_PROD_DEF(psym_declaration_specifiers,
                                PARSER_PUNCTUATOR(punc_semicolon))
        },

        { psym_function_definition,
                PARSER_PROD_DEF(psym_declaration_specifiers,
                                psym_declarator,
                                psym_declaration_list,
                                psym_compound_statement)
        },
        { psym_function_definition,
                PARSER_PROD_DEF(psym_declaration_specifiers,
                                psym_declarator,
                                psym_compound_statement)
        },

        { psym_function_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_inline))
        },

        { psym_storage_class_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_typedef))
        },
        { psym_storage_class_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_extern))
        },
        { psym_storage_class_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_static))
        },
        { psym_storage_class_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_auto))
        },
        { psym_storage_class_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_register))
        },

        { psym_abstract_declarator,
                PARSER_PROD_DEF(psym_pointer)
        },
        { psym_abstract_declarator,
                PARSER_PROD_DEF(psym_pointer,
                                psym_direct_abstract_declarator)
        },
        { psym_abstract_declarator,
                PARSER_PROD_DEF(psym_direct_abstract_declarator)
        },

        { psym_declarator,
                PARSER_PROD_DEF(psym_pointer,
                                psym_direct_declarator)
        },
        { psym_declarator,
                PARSER_PROD_DEF(psym_direct_declarator)
        },

        { psym_external_declaration,
                PARSER_PROD_DEF(psym_function_definition)
        },
        { psym_external_declaration,
                PARSER_PROD_DEF(psym_declaration)
        },

        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_storage_class_specifier,
                                psym_declaration_specifiers)
        },
        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_storage_class_specifier)
        },
        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_type_specifier,
                                psym_declaration_specifiers)
        },
        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_type_specifier)
        },
        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_type_qualifier,
                                psym_declaration_specifiers)
        },
        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_type_qualifier)
        },
        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_function_specifier,
                                psym_declaration_specifiers)
        },
        { psym_declaration_specifiers,
                PARSER_PROD_DEF(psym_function_specifier)
        },

        { psym_parameter_declaration,
                PARSER_PROD_DEF(psym_declaration_specifiers,
                                psym_declarator)
        },
        { psym_parameter_declaration,
                PARSER_PROD_DEF(psym_declaration_specifiers,
                                psym_abstract_declarator)
        },
        { psym_parameter_declaration,
                PARSER_PROD_DEF(psym_declaration_specifiers)
        },

        { psym_parameter_list,
                PARSER_PROD_DEF(psym_parameter_declaration,
                                psym_parameter_list2)
        },
        { psym_parameter_list2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_parameter_declaration,
                                psym_parameter_list2)
        },
        { psym_parameter_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_parameter_type_list, 
                PARSER_PROD_DEF(psym_parameter_list)
        },
        { psym_parameter_type_list, 
                PARSER_PROD_DEF(psym_parameter_list,
                                PARSER_PUNCTUATOR(punc_comma),
                                PARSER_PUNCTUATOR(punc_dots))
        },

        { psym_typedef_name, 
                PARSER_PROD_DEF(psym_identifier)
        },

        { psym_translation_unit,
                PARSER_PROD_DEF(psym_external_declaration,
                                psym_translation_unit2)
        },
        { psym_translation_unit2,
                PARSER_PROD_DEF(psym_external_declaration,
                                psym_translation_unit2)
        },
        { psym_translation_unit2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_struct_declarator_list, 
                PARSER_PROD_DEF(psym_struct_declarator,
                                psym_struct_declarator_list2)
        },
        { psym_struct_declarator_list2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_struct_declarator,
                                psym_struct_declarator_list2)
        },
        { psym_struct_declarator_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_struct_declaration,
                PARSER_PROD_DEF(psym_specifier_qualifier_list,
                                psym_struct_declarator_list,
                                PARSER_PUNCTUATOR(punc_semicolon))
        },

        { psym_struct_declaration_list, 
                PARSER_PROD_DEF(psym_struct_declaration,
                                psym_struct_declaration_list2)
        },
        { psym_struct_declaration_list2, 
                PARSER_PROD_DEF(psym_struct_declaration,
                                psym_struct_declaration_list2)
        },
        { psym_struct_declaration_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_struct_or_union, 
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_struct))
        },
        { psym_struct_or_union, 
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_union))
        },

        { psym_pointer,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mul),
                                psym_type_qualifier_list)
        },
        { psym_pointer,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mul))
        },
        { psym_pointer,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mul),
                                psym_type_qualifier_list,
                                psym_pointer)
        },
        { psym_pointer,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mul), 
                                psym_pointer)
        },

        { psym_struct_or_union_specifier,
                PARSER_PROD_DEF(psym_struct_or_union,
                                psym_identifier,
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_struct_declaration_list,
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },
        { psym_struct_or_union_specifier,
                PARSER_PROD_DEF(psym_struct_or_union,
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_struct_declaration_list,
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },
        { psym_struct_or_union_specifier,
                PARSER_PROD_DEF(psym_struct_or_union,
                                psym_identifier)
        },

        { psym_type_qualifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_const))
        },
        { psym_type_qualifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_restrict))
        },
        { psym_type_qualifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_volatile))
        },

        { psym_direct_abstract_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_abstract_declarator,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_assignment_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_PUNCTUATOR(punc_mul),
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_parameter_type_list,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_PUNCTUATOR(punc_mul),
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_assignment_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_parameter_type_list,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_direct_abstract_declarator2)
        },
        { psym_direct_abstract_declarator2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_abstract_declarator,
                PARSER_PROD_DEF(psym_pointer)
        },
        { psym_abstract_declarator,
                PARSER_PROD_DEF(psym_pointer,
                                psym_direct_abstract_declarator)
        },
        { psym_abstract_declarator,
                PARSER_PROD_DEF(psym_direct_abstract_declarator)
        },

        { psym_argument_expression_list,
                PARSER_PROD_DEF(psym_assignment_expression,
                                psym_argument_expression_list2)
        },
        { psym_argument_expression_list2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_assignment_expression,
                                psym_argument_expression_list2)
        },
        { psym_argument_expression_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_initializer,
                PARSER_PROD_DEF(psym_assignment_expression)
        },
        { psym_initializer,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_initializer_list,
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },
        { psym_initializer,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_initializer_list,
                                PARSER_PUNCTUATOR(punc_comma),
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },

        { psym_designator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br),
                                psym_constant_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br))
        },
        { psym_designator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_dot),
                                psym_identifier)
        },

        { psym_designator_list,
                PARSER_PROD_DEF(psym_designator,
                                psym_designator_list2)
        },
        { psym_designator_list2,
                PARSER_PROD_DEF(psym_designator,
                                psym_designator_list2)
        },
        { psym_designator_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_designation,
                PARSER_PROD_DEF(psym_designator_list,
                                PARSER_PUNCTUATOR(punc_assign))
        },

        { psym_initializer_list,
                PARSER_PROD_DEF(psym_designation,
                                psym_initializer,
                                psym_initializer_list2)
        },
        { psym_initializer_list,
                PARSER_PROD_DEF(psym_initializer,
                                psym_initializer_list2)
        },
        { psym_initializer_list2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_designation,
                                psym_initializer,
                                psym_initializer_list2)
        },
        { psym_initializer_list2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_initializer,
                                psym_initializer_list2)
        },
        { psym_initializer_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_unary_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_bit_and))
        },
        { psym_unary_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mul))
        },
        { psym_unary_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_add))
        },
        { psym_unary_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_sub))
        },
        { psym_unary_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_tilde))
        },
        { psym_unary_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_exclamation))
        },

        { psym_enumeration_constant,
                PARSER_PROD_DEF(psym_identifier)
        },

        { psym_enumerator,
                PARSER_PROD_DEF(psym_enumeration_constant)
        },
        { psym_enumerator,
                PARSER_PROD_DEF(psym_enumeration_constant,
                                PARSER_PUNCTUATOR(punc_assign),
                                psym_constant_expression)
        },

        { psym_enumerator_list,
                PARSER_PROD_DEF(psym_enumerator,
                                psym_enumerator_list2)
        },
        { psym_enumerator_list2,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_enumerator,
                                psym_enumerator_list2)
        },
        { psym_enumerator_list2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_enum_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_enum),
                                psym_identifier,
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_enum_specifier,
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },
        { psym_enum_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_enum),
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_enum_specifier,
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },
        { psym_enum_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_enum),
                                psym_identifier,
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_enum_specifier,
                                PARSER_PUNCTUATOR(punc_comma),
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },
        { psym_enum_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_enum),
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_enum_specifier,
                                PARSER_PUNCTUATOR(punc_comma),
                                PARSER_PUNCTUATOR(punc_right_ql_br))
        },
        { psym_enum_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_enum),
                                psym_identifier)
        },

        { psym_type_name,
                PARSER_PROD_DEF(psym_identifier)
        },

        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_void))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_char))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_short))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_int))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_long))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_float))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_double))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_signed))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_unsigned))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_Bool))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_Complex))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_Imaginary))
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(psym_struct_or_union_specifier)
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(psym_enum_specifier)
        },
        { psym_type_specifier,
                PARSER_PROD_DEF(psym_typedef_name)
        },

        { psym_specifier_qualifier_list, 
                PARSER_PROD_DEF(psym_type_qualifier)
        },
        { psym_specifier_qualifier_list, 
                PARSER_PROD_DEF(psym_type_qualifier,
                                psym_specifier_qualifier_list)
        },
        { psym_specifier_qualifier_list, 
                PARSER_PROD_DEF(psym_type_specifier)
        },
        { psym_specifier_qualifier_list, 
                PARSER_PROD_DEF(psym_type_specifier,
                                psym_specifier_qualifier_list)
        },

        { psym_type_name,
                PARSER_PROD_DEF(psym_specifier_qualifier_list)
        },
        { psym_type_name,
                PARSER_PROD_DEF(psym_specifier_qualifier_list,
                                psym_abstract_declarator)
        },

        { psym_constant_expression, 
                PARSER_PROD_DEF(psym_conditional_expression)
        },

        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mul_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_div_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mod_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_add_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_sub_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_shl_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_shr_assigh))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_and_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_xor_assign))
        },
        { psym_assignment_operator,
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_or_assign))
        },

        { psym_assignment_expression,
                PARSER_PROD_DEF(psym_conditional_expression)
        },
        { psym_assignment_expression,
                PARSER_PROD_DEF(psym_unary_expression,
                                psym_assignment_operator,
                                psym_assignment_expression)
        },

        { psym_expression, 
                PARSER_PROD_DEF(psym_assignment_expression,
                                psym_expression2)
        },
        { psym_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_comma),
                                psym_assignment_expression,
                                psym_expression2)
        },
        { psym_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_primary_expression, 
                PARSER_PROD_DEF(psym_identifier)
        },
        { psym_primary_expression, 
                PARSER_PROD_DEF(psym_constant)
        },
        { psym_primary_expression, 
                PARSER_PROD_DEF(psym_string_literal)
        },
        { psym_primary_expression, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_rnd_br))
        },

        { psym_postfix_expression, 
                PARSER_PROD_DEF(psym_primary_expression,
                                psym_postfix_expression2)
        },
        { psym_postfix_expression, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_type_name,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_initializer_list,
                                PARSER_PUNCTUATOR(punc_right_ql_br),
                                psym_postfix_expression2)
        },
        { psym_postfix_expression, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_type_name,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                PARSER_PUNCTUATOR(punc_left_ql_br),
                                psym_initializer_list,
                                PARSER_PUNCTUATOR(punc_comma),
                                PARSER_PUNCTUATOR(punc_right_ql_br),
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_sq_br), 
                                psym_expression,
                                PARSER_PUNCTUATOR(punc_right_sq_br),
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br), 
                                psym_argument_expression_list,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br), 
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_dot), 
                                psym_identifier,
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_right_arrow), 
                                psym_identifier,
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_increment), 
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_decrement), 
                                psym_postfix_expression2)
        },
        { psym_postfix_expression2,
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_unary_expression, 
                PARSER_PROD_DEF(psym_postfix_expression)
        },
        { psym_unary_expression, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_increment),
                                psym_unary_expression)
        },
        { psym_unary_expression, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_decrement),
                                psym_unary_expression)
        },
        { psym_unary_expression, 
                PARSER_PROD_DEF(psym_unary_operator,
                                psym_cast_expression)
        },
        { psym_unary_expression, 
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_sizeof),
                                psym_unary_expression)
        },
        { psym_unary_expression, 
                PARSER_PROD_DEF(PARSER_KEYWORD(keyw_sizeof),
                                PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_type_name,
                                PARSER_PUNCTUATOR(punc_right_rnd_br))
        },

        { psym_cast_expression, 
                PARSER_PROD_DEF(psym_unary_expression)
        },
        { psym_cast_expression, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_left_rnd_br),
                                psym_type_name,
                                PARSER_PUNCTUATOR(punc_right_rnd_br),
                                psym_cast_expression)
        },

        { psym_multiplicative_expression, 
                PARSER_PROD_DEF(psym_cast_expression, 
                                psym_multiplicative_expression2)
        },
        { psym_multiplicative_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_mul), 
                                psym_cast_expression,
                                psym_multiplicative_expression2)
        },
        { psym_multiplicative_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_forward_slash), 
                                psym_cast_expression,
                                psym_multiplicative_expression2)
        },
        { psym_multiplicative_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_percent), 
                                psym_cast_expression,
                                psym_multiplicative_expression2)
        },
        { psym_multiplicative_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_additive_expression, 
                PARSER_PROD_DEF(psym_multiplicative_expression, 
                                psym_additive_expression2)
        },
        { psym_additive_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_add), 
                                psym_multiplicative_expression,
                                psym_additive_expression2)
        },
        { psym_additive_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_sub), 
                                psym_multiplicative_expression,
                                psym_additive_expression2)
        },
        { psym_additive_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_shift_expression, 
                PARSER_PROD_DEF(psym_additive_expression, 
                                psym_shift_expression2)
        },
        { psym_shift_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_shl), 
                                psym_additive_expression,
                                psym_shift_expression2)
        },
        { psym_shift_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_shr), 
                                psym_additive_expression,
                                psym_shift_expression2)
        },
        { psym_shift_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_relational_expression, 
                PARSER_PROD_DEF(psym_shift_expression, 
                                psym_relational_expression2)
        },
        { psym_relational_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_less), 
                                psym_shift_expression,
                                psym_relational_expression2)
        },
        { psym_relational_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_greater), 
                                psym_shift_expression,
                                psym_relational_expression2)
        },
        { psym_relational_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_less_eq), 
                                psym_shift_expression,
                                psym_relational_expression2)
        },
        { psym_relational_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_greater_eq), 
                                psym_shift_expression,
                                psym_relational_expression2)
        },
        { psym_relational_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_equality_expression, 
                PARSER_PROD_DEF(psym_relational_expression, 
                                psym_equality_expression2)
        },
        { psym_equality_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_equal), 
                                psym_relational_expression,
                                psym_equality_expression2)
        },
        { psym_equality_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_not_equal), 
                                psym_relational_expression,
                                psym_equality_expression2)
        },
        { psym_equality_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_and_expression, 
                PARSER_PROD_DEF(psym_equality_expression, 
                                psym_and_expression2)
        },
        { psym_and_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_bit_and), 
                                psym_equality_expression,
                                psym_and_expression2)
        },
        { psym_and_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_exclusive_or_expression, 
                PARSER_PROD_DEF(psym_and_expression, 
                                psym_exclusive_or_expression2)
        },
        { psym_exclusive_or_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_xor), 
                                psym_and_expression,
                                psym_exclusive_or_expression2)
        },
        { psym_exclusive_or_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_inclusive_or_expression, 
                PARSER_PROD_DEF(psym_exclusive_or_expression, 
                                psym_inclusive_or_expression2)
        },
        { psym_inclusive_or_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_bar), 
                                psym_exclusive_or_expression,
                                psym_inclusive_or_expression2)
        },
        { psym_inclusive_or_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_logical_and_expression, 
                PARSER_PROD_DEF(psym_inclusive_or_expression, 
                                psym_logical_and_expression2)
        },
        { psym_logical_and_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_logical_and), 
                                psym_inclusive_or_expression,
                                psym_logical_or_expression2)
        },
        { psym_logical_and_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },
        
        { psym_logical_or_expression, 
                PARSER_PROD_DEF(psym_logical_and_expression, 
                                psym_logical_or_expression2)
        },
        { psym_logical_or_expression2, 
                PARSER_PROD_DEF(PARSER_PUNCTUATOR(punc_logical_or), 
                                psym_logical_and_expression,
                                psym_logical_or_expression2)
        },
        { psym_logical_or_expression2, 
                PARSER_PROD_DEF(psym_epsilon)
        },

        { psym_conditional_expression, 
                PARSER_PROD_DEF(psym_logical_or_expression)
        },
        { psym_conditional_expression, 
                PARSER_PROD_DEF(psym_logical_or_expression, 
                                PARSER_PUNCTUATOR(punc_question),
                                psym_expression, 
                                PARSER_PUNCTUATOR(punc_colon),
                                psym_conditional_expression) 
        },
};

size_t parser_grammar_size()
{
        return sizeof(parser_grammar) / sizeof(struct parser_production);
}