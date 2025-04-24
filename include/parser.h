#ifndef _PARSER_H_
#define _PARSER_H_
#include <stdbool.h>

#include <list.h>
#include <parser/symbol.h>
#include <parser/ast.h>
#include <parser/symtable.h>
#include <mc.h>
#include <stack.h>

#define PARSER_STACK_CAPACITY 25

struct parser_clb {
        struct token*           (*pull_token)(void *data);
        void                    (*put_token)(void *data, struct token *tok);
        struct token*           (*fetch_token)(void *data);
        mc_status_t             (*error)(void *data, const char *message);
};

struct parser;

typedef mc_status_t (*parser_process_node_t)(struct parser *ps);

struct parser_ops {
        parser_process_node_t   primary_expression;
        parser_process_node_t   postfix_expression;
        parser_process_node_t   argument_expression_list;
        parser_process_node_t   unary_expression;
        parser_process_node_t   cast_expression;
        parser_process_node_t   multiplicative_expression;
        parser_process_node_t   additive_expression;
        parser_process_node_t   shift_expression;
        parser_process_node_t   relational_expression;
        parser_process_node_t   equality_expression;
        parser_process_node_t   and_expression;
        parser_process_node_t   exclusive_or_expression;
        parser_process_node_t   inclusive_or_expression;
        parser_process_node_t   logical_and_expression;
        parser_process_node_t   logical_or_expression;
        parser_process_node_t   conditional_expression;
        parser_process_node_t   assignment_expression;
        parser_process_node_t   expression;
        parser_process_node_t   constant_expression;
        parser_process_node_t   declaration;
        parser_process_node_t   declaration_specifiers;
        parser_process_node_t   init_declarator_list;
        parser_process_node_t   init_declarator;
        parser_process_node_t   type_specifier;
        parser_process_node_t   struct_or_union_specifier;
        parser_process_node_t   enum_specifier;
        parser_process_node_t   declarator;
        parser_process_node_t   direct_declarator;
        parser_process_node_t   pointer;
        parser_process_node_t   type_qualifier_list;
        parser_process_node_t   parameter_type_list;
        parser_process_node_t   parameter_list;
        parser_process_node_t   parameter_declaration;
        parser_process_node_t   identifier_list;
        parser_process_node_t   type_name;
        parser_process_node_t   abstract_declarator;
        parser_process_node_t   direct_abstract_declarator;
        parser_process_node_t   initializer;
        parser_process_node_t   initializer_list;
        parser_process_node_t   designation;
        parser_process_node_t   designator_list;
        parser_process_node_t   designator;
        parser_process_node_t   statement;
        parser_process_node_t   labeled_statement;
        parser_process_node_t   compound_statement;
        parser_process_node_t   block_item_list;
        parser_process_node_t   block_item;
        parser_process_node_t   expression_statement;
        parser_process_node_t   selection_statement;
        parser_process_node_t   iteration_statement;
        parser_process_node_t   jump_statement;
        parser_process_node_t   translation_unit;
        parser_process_node_t   external_declaration;
        parser_process_node_t   function_definition;
};

struct parser {
        /* is used to store top level unfinished production */
        struct stack stack;
        /* for fetched lookahed production */
        struct pt_node *lookahead;
        struct parser_clb clb;
        void *data; /* some user provided data, is passed to ops */
        struct symtable sym_tbl; 
        const struct parser_ops *ops;
};

struct pt_node *parser_result_pull(struct parser *ps);

enum parser_symbol parser_token_tosymbol(struct token *tok);

void parser_init(struct parser *ps, struct parser_clb clb, void *user_data);

void parser_free(struct parser *ps);

#endif /* _PARSER_H_ */