#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <parser.h>
#include <parser/tree.h>
#include "symset.h"
#include "lookahead.h"

static mc_status_t parser_struct_or_union_specifier(struct parser *ps);
static mc_status_t parser_enum_specifier(struct parser *ps);
static mc_status_t parser_conditional_expression(struct parser *ps);
static mc_status_t parser_cast_expression(struct parser *ps);
static mc_status_t parser_assignment_expression(struct parser *ps);
static mc_status_t parser_expression(struct parser *ps);
static mc_status_t parser_constant_expression(struct parser *ps);
static mc_status_t parser_specifier_qualifier_list(struct parser *ps);
static mc_status_t parser_abstract_declarator(struct parser *ps);
static mc_status_t parser_pointer(struct parser *ps);
static mc_status_t parser_declaration_specifiers(struct parser *ps);
static mc_status_t parser_initializer_list(struct parser *ps);
static mc_status_t parser_declaration(struct parser *ps);
static mc_status_t parser_compound_statement(struct parser *ps);
static mc_status_t parser_statement(struct parser *ps);
static mc_status_t parser_type_qualifier_list(struct parser *ps);
static mc_status_t parser_declarator(struct parser *ps);
static mc_status_t parser_parameter_type_list(struct parser *ps);

static inline void parser_stack_init(struct parser *ps)
{
        stack_init(&ps->stack, PARSER_STACK_CAPACITY, 
                sizeof(struct pt_node *));
}

static inline 
void parser_stack_push(struct parser *ps, struct pt_node *node)
{
        stack_push(&ps->stack, &node);
}

static inline struct pt_node* 
parser_stack_top(struct parser *ps)
{
        struct pt_node *result;
        stack_top(&ps->stack, &result);
        return result;
}

static inline
struct pt_node *parser_stack_pop(struct parser *ps)
{

        struct stack *stack = &ps->stack;
        struct pt_node *top;
        stack_top(stack, &top);
        stack_pop(stack);
        return top;
}

/* remove and free all unfinished productions from the stack, up
 * untill root @failed one */
static inline 
void parser_stack_restore(struct parser *ps, struct pt_node *failed)
{
        struct pt_node *top;
        do {
                top = parser_stack_pop(ps);
                pt_node_destroy(top);
        } while (top != failed);
}

static inline 
void parser_stack_free(struct parser *ps)
{
        struct stack *st = &ps->stack;
        while (st->size != 0)
                pt_node_destroy(parser_stack_pop(ps));
        stack_free(st);
}

enum parser_symbol parser_token_tosymbol(struct token *tok)
{
        switch (tok->type) {
                case tok_keyword:
                        return psym_first_keyw + tok->value.var_keyw;
                case tok_identifier:
                        return psym_identifier;
                case tok_constant:
                        return psym_constant;
                case tok_strlit:
                        return psym_string_literal;
                case tok_punctuator:
                        return psym_first_punc + tok->value.var_punc;
                default:
                        MC_LOG(MC_CRIT, "unexpected token type");
                        break;
        }
        return psym_invalid;
}

static inline struct token*
parser_pull_token(struct parser *ps)
{
        struct parser_ops ops = ps->ops; 
        struct token *next = ops.fetch_token(ps->data);
        ops.pull_token(ps->data);
        return next;
}

static inline void parser_put_token(struct parser *ps, struct token *tok)
{
        struct parser_ops ops = ps->ops; 
        ops.put_token(ps->data, tok);
}

static inline struct token*
parser_fetch_token(struct parser *ps)
{
        struct parser_ops ops = ps->ops; 
        return ops.fetch_token(ps->data);
}

static inline enum parser_symbol
parser_fetch_symbol(struct parser *ps)
{
        struct token *tok = parser_fetch_token(ps);
        if (tok != NULL)
                return parser_token_tosymbol(tok);
        else 
                return psym_invalid;
}

enum parser_symbol parser_get_symbol(struct parser *ps, 
                                     struct token *name)
{
        struct parser_ops ops = ps->ops; 
        return ops.get_symbol(ps->data, name);
}

static inline _Bool
parser_error(struct parser *ps, char *message)
{
        return ps->ops.error(ps->data, message);
}

static mc_status_t parser_direct_declarator_array(struct parser *ps, 
                                                  struct pt_node *dir_decl)
{
        mc_status_t status;
        static const uint8_t type_lst_first[] = { TYPE_QUALIFIER_LIST };
        static const uint8_t assig_first[] = { ASSIGNMENT_EXPRESSION };

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (type_lst_first[sym]) {
                status = parser_type_qualifier_list(ps);
                if (!MC_SUCC(status)) {
                        return parser_error(ps, "invalid type qualifier list");
                }
                pt_node_child_add(dir_decl, parser_stack_pop(ps));
        } else if (sym == PARSER_KEYWORD(keyw_static)) {
                pt_node_child_add(dir_decl, 
                        pt_node_create_leaf(parser_pull_token(ps)));

                if (type_lst_first[parser_fetch_symbol(ps)]) {
                        status = parser_type_qualifier_list(ps);
                        if (!MC_SUCC(status)) {
                                return parser_error(ps, 
                                        "invalid type qualifier list");
                        }
                        pt_node_child_add(dir_decl, parser_stack_pop(ps));
                }

                if (assig_first[parser_fetch_symbol(ps)]) {
                        return parser_error(ps, 
                                "expected assignment expression");
                }
                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status)) {
                        return parser_error(ps, 
                                "invalid assignment expression");
                }
                pt_node_child_add(dir_decl, parser_stack_pop(ps));
                return MC_OK;
        } else if (sym == PARSER_PUNCTUATOR(punc_mul)) {
                pt_node_child_add(dir_decl, 
                        pt_node_create_leaf(parser_pull_token(ps)));
                return MC_OK;
        } else if (assig_first[sym]) {
                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status)) {
                        return parser_error(ps, 
                                "invalid assignment expression");
                }
                pt_node_child_add(dir_decl, parser_stack_pop(ps));
                return MC_OK;
        } else {
                return MC_OK;
        }

        sym = parser_fetch_symbol(ps);
        if (assig_first[sym]) {
                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status)) {
                        return parser_error(ps, 
                                "invalid assignment expression");
                }
                pt_node_child_add(dir_decl, parser_stack_pop(ps));
        } else if (sym == PARSER_KEYWORD(keyw_static)) {
                pt_node_child_add(dir_decl, 
                        pt_node_create_leaf(parser_pull_token(ps)));

                if (!assig_first[parser_fetch_symbol(ps)]) {
                        return parser_error(ps, 
                                "expected assignment expression");
                }

                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid assignment expression");
                        return MC_FAIL;
                }
                pt_node_child_add(dir_decl, parser_stack_pop(ps));
        } else if (sym == PARSER_KEYWORD(punc_mul)) {
                pt_node_child_add(dir_decl, 
                        pt_node_create_leaf(parser_pull_token(ps)));
        }

        return MC_OK;
}

static mc_status_t parser_identifier_list(struct parser *ps)
{
        mc_status_t status;
        struct pt_node *id_lst = pt_node_create(psym_identifier_list);
        parser_stack_push(ps, id_lst);

        do {
                if (parser_fetch_symbol(ps) != psym_identifier) {
                        status = parser_error(ps, "expected identifier");
                        goto fail;
                }
                pt_node_child_add(id_lst, 
                        pt_node_create_leaf(parser_pull_token(ps)));

        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, id_lst);
        return status;
}

static mc_status_t parser_direct_declarator_parameter(struct parser *ps, 
                                                      struct pt_node *dir_decl)
{
        mc_status_t status;
        static const uint8_t param_first[] = { PARAMETER_TYPE_LIST };
        enum parser_symbol sym = parser_fetch_symbol(ps);

        _Bool is_id = (sym == psym_identifier);
        if ((param_first[sym] && !is_id) || (is_id && parser_get_symbol(ps,
                parser_fetch_token(ps)) != psym_typedef_name)) {
                status = parser_parameter_type_list(ps);
                if (!MC_SUCC(status))
                        return parser_error(ps, "invalid parameter type list");
                pt_node_child_add(dir_decl, parser_stack_pop(ps));
        } else if (is_id) {
                status = parser_identifier_list(ps);
                if (!MC_SUCC(status))
                        return parser_error(ps, "invalid identifier list");
                pt_node_child_add(dir_decl, parser_stack_pop(ps));
        } 

        return MC_OK;
}

static mc_status_t parser_direct_declarator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t decl_first[] = { DECLARATOR };
        struct pt_node *dir_decl = pt_node_create(psym_direct_declarator);
        parser_stack_push(ps, dir_decl);

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (sym == psym_identifier) {
                pt_node_child_add(dir_decl, 
                        pt_node_create_leaf(parser_pull_token(ps)));
        } else if (sym == PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                /* pull the ( before ( declarator ) */
                parser_pull_token(ps);

                if (!decl_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected declarator");
                        goto fail;
                }
                status = parser_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid declarator");
                        goto fail;
                }
                pt_node_child_add(dir_decl, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) 
                        != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                        status = parser_error(ps, "invalid declarator");
                        goto fail;
                }
                parser_pull_token(ps);
        } else {
                status = parser_error(ps, 
                        "expected ( declarator ) or identifier");
                goto fail;
        }

        for (sym = parser_fetch_symbol(ps);
                sym == PARSER_PUNCTUATOR(punc_left_sq_br) 
                || sym == PARSER_PUNCTUATOR(punc_left_rnd_br);
                sym = parser_fetch_symbol(ps)) {

                /* pull [ or ( */
                parser_pull_token(ps);

                if (sym == PARSER_PUNCTUATOR(punc_left_sq_br)) {
                        status = parser_direct_declarator_array(ps, dir_decl);
                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_right_sq_br)) {
                                status = parser_error(ps, "expected ] symbol");
                                goto fail;
                        }
                } else if (sym == PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                        status = parser_direct_declarator_parameter(ps, 
                                dir_decl);
                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                                status = parser_error(ps, "expected ) symbol");
                                goto fail;
                        }
                }

                /* pull ] or ) */
                parser_pull_token(ps);
        }
fail:
        parser_stack_restore(ps, dir_decl);
        return status;        
}

static mc_status_t parser_declarator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t ptr_first[] = { POINTER };
        static const uint8_t dir_decl_first[] = { DIRECT_DECLARATOR };
        struct pt_node *decl = pt_node_create(psym_declarator);
        parser_stack_push(ps, decl);

        if (ptr_first[parser_fetch_symbol(ps)]) {
                status = parser_pointer(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid pointer");
                        goto fail;
                }
                pt_node_child_add(decl, parser_stack_pop(ps));       
        }

        if (!dir_decl_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected direct declarator");
                goto fail;
        }

        status = parser_direct_declarator(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid direct declarator");
                goto fail;
        }
        pt_node_child_add(decl, parser_stack_pop(ps));       

        return MC_OK;
fail:
        parser_stack_restore(ps, decl);
        return status;
}

static mc_status_t parser_enumeration_constant(struct parser *ps)
{
        struct pt_node *enum_const = pt_node_create(psym_enumeration_constant);
        parser_stack_push(ps, enum_const);
        pt_node_child_add(enum_const, pt_node_create_leaf(
                parser_pull_token(ps)));
        return MC_OK;
}

static mc_status_t parser_enumerator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t const_expr_first[] = { CONSTANT_EXPRESSION };
        struct pt_node *enumerator = pt_node_create(psym_enumerator);
        parser_stack_push(ps, enumerator);

        status = parser_enumeration_constant(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid enumeration constant");
                goto fail;
        }
        pt_node_child_add(enumerator, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_assign)) {
                parser_pull_token(ps);

                if (!const_expr_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected constant expression");
                        goto fail;
                }
                status = parser_constant_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid constant expression");
                        goto fail;
                }
                pt_node_child_add(enumerator, parser_stack_pop(ps));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, enumerator);
        return status;
}

static mc_status_t parser_enumerator_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t enum_first[] = { ENUMERATOR };
        struct pt_node *enum_lst = pt_node_create(psym_enumerator_list);
        parser_stack_push(ps, enum_lst);

        do {
                if (!enum_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected enumerator");
                        goto fail;
                }

                status = parser_enumerator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid enumerator");
                        goto fail;
                }
                pt_node_child_add(enum_lst, parser_stack_pop(ps));

        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, enum_lst);
        return status;
}

static mc_status_t parser_enum_specifier(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t enum_lst[] = { ENUMERATOR_LIST };
        struct pt_node *enum_spec = pt_node_create(psym_enum_specifier);
        parser_stack_push(ps, enum_spec);

        /* pull the enum keyword */
        parser_pull_token(ps);

        if (parser_fetch_symbol(ps) == psym_identifier) {
                pt_node_child_add(enum_spec, 
                        pt_node_create_leaf(parser_pull_token(ps)));
        }

        if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_left_ql_br)) {
                parser_pull_token(ps);

                if (!enum_lst[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected enumerator list");
                        goto fail;
                }
                status = parser_enumerator_list(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid enumerator list");
                        goto fail;
                }
                pt_node_child_add(enum_spec, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) 
                        == PARSER_PUNCTUATOR(punc_comma))
                        parser_pull_token(ps);
                
                if (parser_fetch_symbol(ps) 
                        != PARSER_PUNCTUATOR(punc_right_ql_br)) {
                        status = parser_error(ps, "expected } symbol");
                        goto fail;
                }
                parser_pull_token(ps);

        } else if (pt_node_child_empty(enum_spec)) {
                status = parser_error(ps, "expected { enumerator-list }");
                goto fail;
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, enum_spec);
        return status;
}

static mc_status_t parser_typedef_name(struct parser *ps)
{

        struct token *tok_id = parser_pull_token(ps);
        assert(parser_get_symbol(ps, tok_id) == psym_typedef_name);
        struct pt_node *tf_name = pt_node_create(psym_type_name);
        parser_stack_push(ps, tf_name);
        pt_node_child_add(tf_name, pt_node_create_leaf(tok_id));
        return MC_OK;
}

static mc_status_t parser_type_specifier(struct parser *ps)
{
        static const uint8_t str_or_un[] = { STRUCT_OR_UNION_SPECIFIER };
        static const uint8_t enum_spec[] = { ENUM_SPECIFIER };
        static const uint8_t tdef_spec[] = { TYPEDEF_NAME };
        struct pt_node *val;
        mc_status_t status;
        struct pt_node *spec = pt_node_create(psym_type_specifier);
        parser_stack_push(ps, spec);
        enum parser_symbol sym = parser_fetch_symbol(ps);

        if (str_or_un[sym]) {
                status = parser_struct_or_union_specifier(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "expected struct or union");
                        goto fail;
                }
        } else if (enum_spec[sym]) {
                status = parser_enum_specifier(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "expected enum specifier");
                        goto fail;
                }
        } else if (tdef_spec[sym]) {
                status = parser_typedef_name(ps);
                assert(MC_SUCC(status));
        } else {
                val = pt_node_create_leaf(parser_pull_token(ps));
                parser_stack_push(ps, val);
        }
        pt_node_child_add(spec, parser_stack_pop(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, spec);
        return status;
}

static mc_status_t parser_type_qualifier(struct parser *ps)
{
        struct pt_node *qual = pt_node_create(psym_type_qualifier);
        struct pt_node *keyw = pt_node_create_leaf(parser_pull_token(ps));
        pt_node_child_add(qual, keyw);
        parser_stack_push(ps, qual);
        return MC_OK;
}

static mc_status_t parser_storage_class_specifier(struct parser *ps)
{
        struct pt_node *spec = pt_node_create(psym_storage_class_specifier);
        struct pt_node *val = pt_node_create_leaf(parser_pull_token(ps));
        pt_node_child_add(spec, val);
        parser_stack_push(ps, spec);
        return MC_OK;
}

static mc_status_t parser_struct_or_union(struct parser *ps)
{
        struct pt_node *st_or_u = pt_node_create(
                psym_struct_or_union);
        struct pt_node *val = pt_node_create_leaf(parser_pull_token(ps));
        pt_node_child_add(st_or_u, val);
        parser_stack_push(ps, val);
        return MC_OK;
}

static mc_status_t parser_argument_expression_list(struct parser *ps)
{
        mc_status_t status = MC_FAIL;
        static const uint8_t assig_expr[] = { ASSIGNMENT_EXPRESSION };
        struct pt_node *expr_list = pt_node_create(
                psym_argument_expression_list);
        parser_stack_push(ps, expr_list);

        do {
                if (!assig_expr[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected assignment expresiion");
                        goto fail;
                }
                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid assignment expresiion");
                        goto fail;
                }
                pt_node_child_add(expr_list, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, expr_list);
        return status;
}

static mc_status_t parser_type_qualifier_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t type_q_first[] = { TYPE_QUALIFIER };
        struct pt_node *q_lst = pt_node_create(psym_type_qualifier_list);
        parser_stack_push(ps, q_lst);

        do {
                status = parser_type_qualifier(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid type qualifier");
                        goto fail;
                }
                pt_node_child_add(q_lst, parser_stack_pop(ps));       
        } while (type_q_first[parser_fetch_symbol(ps)]);

        return MC_OK;
fail:
        parser_stack_restore(ps, q_lst);
        return status;
}

static mc_status_t parser_pointer(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t q_lst_first[] = { TYPE_QUALIFIER_LIST };
        struct pt_node *ptr = pt_node_create(psym_pointer);
        parser_stack_push(ps, ptr);

        do {
                /* pull * symbol */
                parser_pull_token(ps);

                if (q_lst_first[parser_fetch_symbol(ps)]) {
                        status = parser_type_qualifier_list(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, 
                                        "invalid type qualifier list");
                                goto fail;
                        }
                        pt_node_child_add(ptr, parser_stack_pop(ps));       
                }

        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma));

        return MC_OK;
fail:
        parser_stack_restore(ps, ptr);
        return status;
}

/* symbol ) or * in center of abstract or pure declarator */
static enum parser_symbol parser_fetch_declarator_center(struct parser *ps)
{
        struct stack tok_stack;
        struct token *tok;
        enum parser_symbol sym;
        stack_init(&tok_stack, PARSER_STACK_CAPACITY, sizeof(struct token *));
        do {
                tok = parser_pull_token(ps);
                stack_push(&tok_stack, &tok);
        } while ((sym = parser_fetch_symbol(ps)) 
                == PARSER_PUNCTUATOR(punc_left_rnd_br)
                || sym == PARSER_PUNCTUATOR(punc_mul));

        while (tok_stack.size != 0) {
                stack_top(&tok_stack, &tok);
                parser_put_token(ps, tok);
                stack_pop(&tok_stack);
        }
        return sym;
}

static mc_status_t parser_parameter_declaration(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t declarator_first[] = { DECLARATOR };
        static const uint8_t abs_decltor_first[] = { ABSTRACT_DECLARATOR };
        struct pt_node *param_decl = pt_node_create(psym_parameter_declaration);
        parser_stack_push(ps, param_decl);

        status = parser_declaration_specifiers(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, 
                        "invalid declaration specifiers");
                goto fail;
        }
        pt_node_child_add(param_decl, parser_stack_pop(ps));

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (!declarator_first[sym] && !abs_decltor_first[sym])
                /* abstract declarator is opt so we finish */
                return MC_OK;

        /* only abstract declarator has first [ here */
        if (sym == PARSER_PUNCTUATOR(punc_left_sq_br)) {
                status = parser_abstract_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid abstract declarator");
                        goto fail;
                }
                pt_node_child_add(param_decl, parser_stack_pop(ps));
                return MC_OK;
        }

        /* go ahead, skipping * and ( , until we meet id or ), 
         * to differ declarator from abstract declarator */
        sym = parser_fetch_declarator_center(ps);

        if (sym == psym_identifier) {

                status = parser_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid declarator");
                        goto fail;
                }
                pt_node_child_add(param_decl, parser_stack_pop(ps));
        } else if (sym == PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                status = parser_abstract_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid abstract declarator");
                        goto fail;
                }
                pt_node_child_add(param_decl, parser_stack_pop(ps));
        } else {
                status = parser_error(ps, 
                        "expected ) symbol or identifier");
                goto fail;
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, param_decl);
        return status;
}

static mc_status_t parser_parameter_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t param_dec_first[] = { PARAMETER_DECLARATION };
        struct pt_node *param_lst = pt_node_create(psym_parameter_list);
        parser_stack_push(ps, param_lst);

        do {
                if (!param_dec_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected parameter declaration");
                        goto fail;
                }

                status = parser_parameter_declaration(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid parameter declaration");
                        goto fail;
                }
                pt_node_child_add(param_lst, parser_stack_pop(ps));       
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, param_lst);
        return status;
}

static mc_status_t parser_parameter_type_list(struct parser *ps)
{
        mc_status_t status;
        struct pt_node *pt_lst = pt_node_create(psym_parameter_type_list);
        parser_stack_push(ps, pt_lst);

        status = parser_parameter_list(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid parameter list");
                goto fail;
        }
        pt_node_child_add(pt_lst, parser_stack_pop(ps));       

        if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma)) {
                parser_pull_token(ps);

                if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_dots)) {
                        status = parser_error(ps, "expected ... after ,");
                        goto fail;
                }
                pt_node_child_add(pt_lst, 
                        pt_node_create_leaf(parser_pull_token(ps)));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, pt_lst);
        return status;
}

static mc_status_t parser_direct_abstract_declarator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t abs_decl_first[] = { ABSTRACT_DECLARATOR };
        static const uint8_t d_abs_decl_first[] = { DIRECT_ABSTRACT_DECLARATOR };
        static const uint8_t assig_expr_first[] = { ASSIGNMENT_EXPRESSION };
        static const uint8_t param_lst_first[] = { PARAMETER_TYPE_LIST };
        struct pt_node *abs_decl 
                = pt_node_create(psym_direct_abstract_declarator);
        parser_stack_push(ps, abs_decl);

        /* pull the ( OR [ */
        struct token *first = parser_pull_token(ps);

        if (abs_decl_first[parser_fetch_symbol(ps)]) {
                if (first->type != PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                        status = parser_error(ps, 
                                "unexpected [ before abstract declarator");
                        goto fail;
                }

                status = parser_abstract_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid abstract declarator");
                        goto fail;
                }
                pt_node_child_add(abs_decl, parser_stack_pop(ps));       

                if (parser_fetch_symbol(ps) 
                        != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                        status = parser_error(ps, "expected ) symbol");
                        goto fail;
                }
                parser_pull_token(ps);
        }

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (assig_expr_first[sym]) {
                        if (first->type != PARSER_PUNCTUATOR(punc_left_sq_br)) {
                                status = parser_error(ps, 
                                        "unexpected ( before assignment expression");
                                goto fail;
                        }
                        status = parser_assignment_expression(ps); 
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, 
                                        "invalid assignment expression");
                                goto fail;
                        }
                        pt_node_child_add(abs_decl, parser_stack_pop(ps));       

                        if (parser_fetch_symbol(ps)
                                != PARSER_PUNCTUATOR(punc_right_sq_br)) {
                                status = parser_error(ps, 
                                        "missing ] from [assignment-expression]");
                                goto fail;
                        }
                        parser_pull_token(ps);
                } else if (param_lst_first[sym]) {
                        if (first->type != PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                                status = parser_error(ps, 
                                        "unexpected [ before parameter type list");
                                goto fail;
                        }
                        status = parser_parameter_type_list(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, 
                                        "invalid parameter type list");
                                goto fail;
                        }
                        pt_node_child_add(abs_decl, parser_stack_pop(ps));       

                        if (parser_fetch_symbol(ps)
                                != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                                status = parser_error(ps, 
                                        "missing ) from (parameter-type-list)");
                                goto fail;
                        }
                        parser_pull_token(ps);

                } else if (sym == PARSER_PUNCTUATOR(punc_mul)) {
                        if (first->type != PARSER_PUNCTUATOR(punc_left_sq_br)) {
                                status = parser_error(ps, 
                                        "unexpected ( before * ");
                                goto fail;
                        }
                        /* add * token */
                        pt_node_child_add(abs_decl, 
                                pt_node_create_leaf(parser_pull_token(ps)));

                        if (parser_fetch_symbol(ps)
                                != PARSER_PUNCTUATOR(punc_right_sq_br)) {
                                status = parser_error(ps, "missing ] from [*]");
                                goto fail;
                        }
                        parser_pull_token(ps);
                } else if (sym == PARSER_PUNCTUATOR(punc_right_sq_br)) {
                        if (first->type != PARSER_PUNCTUATOR(punc_left_sq_br)) {
                                status = parser_error(ps, 
                                        "unexpected ( before ] ");
                                goto fail;
                        }
                        /* add [ */
                        pt_node_child_add(abs_decl, pt_node_create_leaf(first));
                        /* and ] */
                        pt_node_child_add(abs_decl, 
                                pt_node_create_leaf(parser_pull_token(ps)));
                } else if (sym == PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                        if (first->type != PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                                status = parser_error(ps, 
                                        "unexpected [ )");
                                goto fail;
                        }
                        /* add ( */
                        pt_node_child_add(abs_decl, pt_node_create_leaf(first));
                        /* and ) */
                        pt_node_child_add(abs_decl, 
                                pt_node_create_leaf(parser_pull_token(ps)));
                } else {
                        status = parser_error(ps, "expected at least ] or )");
                        goto fail;
                }
        } while (d_abs_decl_first[parser_fetch_symbol(ps)] 
                && (first = parser_pull_token(ps)));

        return MC_OK;
fail:
        parser_stack_restore(ps, abs_decl);
        return status;
}

static mc_status_t parser_abstract_declarator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t pointer_first[] = { POINTER };
        static const uint8_t dir_abs_decl_first[] 
                = { DIRECT_ABSTRACT_DECLARATOR };
        struct pt_node *abs_decl = pt_node_create(psym_abstract_declarator);
        parser_stack_push(ps, abs_decl);

        if (pointer_first[parser_fetch_symbol(ps)]) {
                status = parser_pointer(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid pointer");
                        goto fail;
                }
                pt_node_child_add(abs_decl, parser_stack_pop(ps));       
        } 

        if (dir_abs_decl_first[parser_fetch_symbol(ps)]) {
                status = parser_direct_abstract_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid direct abstract declarator");
                        goto fail;
                }
                pt_node_child_add(abs_decl, parser_stack_pop(ps));       

        } else if (pt_node_child_empty(abs_decl)) {
                status = parser_error(ps, 
                        "direct abstract declarator expected");
                goto fail;
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, abs_decl);
        return status;
}

static mc_status_t parser_type_name(struct parser *ps)
{
        static const uint8_t abs_decl_first[] = { ABSTRACT_DECLARATOR };
        struct pt_node *type_name = pt_node_create(psym_type_name);
        parser_stack_push(ps, type_name);

        mc_status_t status = parser_specifier_qualifier_list(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, 
                        "invalid specifier qualifier list");
                goto fail;
        }
        pt_node_child_add(type_name, parser_stack_pop(ps));

        if (abs_decl_first[parser_fetch_symbol(ps)]) {
                status = parser_abstract_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid abstract declarator");
                        goto fail;
                }
                pt_node_child_add(type_name, parser_stack_pop(ps));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, type_name);
        return status;
}

static mc_status_t parser_designator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t const_first[] = { CONSTANT_EXPRESSION };
        struct pt_node *desig = pt_node_create(psym_designator);
        parser_stack_push(ps, desig);

        if (parser_pull_token(ps)->type 
                == PARSER_PUNCTUATOR(punc_left_sq_br)) {

                if (!const_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected constant expression");
                        goto fail;
                }
                status = parser_constant_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid constant expression");
                        goto fail;
                }
                pt_node_child_add(desig, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) 
                        != PARSER_PUNCTUATOR(punc_left_sq_br)) {
                        status = parser_error(ps, "expected ] symbol");
                }
                parser_pull_token(ps);
        } else {
                if (parser_fetch_symbol(ps) != psym_identifier) {
                        status = parser_error(ps, "expected identifier");
                        goto fail;
                }
                pt_node_child_add(desig, 
                        pt_node_create_leaf(parser_pull_token(ps)));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, desig);
        return status;
}

static mc_status_t parser_designator_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t desig_first[] = { DESIGNATOR };
        struct pt_node *desig_lst = pt_node_create(psym_designator_list);
        parser_stack_push(ps, desig_lst);

        do {
                status = parser_designator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid designator");
                        goto fail;
                }
                pt_node_child_add(desig_lst, parser_stack_pop(ps));
        } while (desig_first[parser_fetch_symbol(ps)]);

        return MC_OK; 
fail:
        parser_stack_restore(ps, desig_lst);
        return status;
}

static mc_status_t parser_designation(struct parser *ps)
{
        struct pt_node *designation = pt_node_create(psym_designation);
        parser_stack_push(ps, designation);

        mc_status_t status = parser_designator_list(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid designator");
                goto fail;
        }
        pt_node_child_add(designation, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_assign)) {
                status = parser_error(ps, "expected = symbol");
                goto fail;
        }
        parser_pull_token(ps);

        return MC_OK;
fail:
        parser_stack_restore(ps, designation);
        return status;
}

static mc_status_t parser_initializer(struct parser *ps)
{
        mc_status_t status;
        struct pt_node *init = pt_node_create(psym_initializer);
        parser_stack_push(ps, init);

        if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_left_ql_br)) {
                parser_pull_token(ps);
                status = parser_initializer_list(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid initializer list");
                        goto fail; 
                }
                pt_node_child_add(init, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma))
                        parser_pull_token(ps);

                if (parser_fetch_symbol(ps) 
                        == PARSER_PUNCTUATOR(punc_right_ql_br)) {
                        status = parser_error(ps, "expected } symbol");
                        goto fail; 
                }
                parser_pull_token(ps);
        } else {
                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid assignment expr");
                        goto fail; 
                }
                pt_node_child_add(init, parser_stack_pop(ps));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, init);
        return MC_FAIL;
}

static mc_status_t parser_initializer_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t desig_first[] = { DESIGNATION };
        static const uint8_t initializer_first[] = { INITIALIZER };
        struct pt_node *init_lst = pt_node_create(psym_initializer_list);
        parser_stack_push(ps, init_lst);

        do {
                if (desig_first[parser_fetch_symbol(ps)]) {
                        status = parser_designation(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, 
                                        "invalid designation");
                                goto fail;
                        }
                        pt_node_child_add(init_lst, parser_stack_pop(ps));
                }

                if (!initializer_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected inializer");
                        goto fail;
                }

                status = parser_initializer(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid initializer");
                        goto fail;
                }
                pt_node_child_add(init_lst, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, init_lst);
        return status;
}

static mc_status_t parser_primary_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t expr_first[] = { EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_primary_expression);
        parser_stack_push(ps, expr);

        struct token *first = parser_pull_token(ps);
        if (first->type == PARSER_PUNCTUATOR(punc_left_rnd_br)) {

                if (!expr_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected expression");
                        goto fail;
                }
                status = parser_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid expression");
                        goto fail;
                }
                pt_node_child_add(expr, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) 
                        != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                        status = parser_error(ps, "expected ) symbol");
                        goto fail;
                }
                parser_pull_token(ps);
        } else {
                pt_node_child_add(expr, pt_node_create_leaf(first));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, expr);
        return status;
}

/* note: parsed type-name could be in lookahead  */
static mc_status_t parser_postfix_expression(struct parser *ps)
{
        mc_status_t status = MC_FAIL;
        static const uint8_t postfix_second[] = { POSTFIX_EXPRESSION_SECOND };
        static const uint8_t type_name_first[] = { TYPE_NAME };
        static const uint8_t init_lst_first[] = { INITIALIZER_LIST };
        static const uint8_t expr_first[] = { EXPRESSION };
        static const uint8_t arg_expr_lst[] = { ARGUMENT_EXPRESSION_LIST };
        struct pt_node *expr = parser_lookahead_pull(ps, psym_postfix_expression);
        struct token *tok;
        if (expr == NULL)
                expr = pt_node_create(psym_postfix_expression);
        parser_stack_push(ps, expr);

        /* we already have type name from lookahead */
        _Bool is_parsed_type_name = !pt_node_child_empty(expr);
        _Bool could_parse_type_name = (tok = parser_pull_token(ps))->type 
                == PARSER_PUNCTUATOR(punc_left_rnd_br) 
                && type_name_first[parser_fetch_symbol(ps)];

        if (could_parse_type_name || is_parsed_type_name) {

                /* parse type name if not in lookahead */
                if (!is_parsed_type_name) {
                        status = parser_type_name(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, "invalid type name");
                                goto fail;
                        }
                        pt_node_child_add(expr, parser_stack_pop(ps));

                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                                status = parser_error(ps, "expected ) symbol");
                                goto fail;
                        }
                        parser_pull_token(ps);
                }
                /* parse { initializer list }*/
                if (parser_fetch_symbol(ps) 
                        != PARSER_PUNCTUATOR(punc_left_ql_br)) {
                        status = parser_error(ps, "expected { symbol");
                        goto fail;
                }
                parser_pull_token(ps);

                if (!init_lst_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected initializer list");
                        goto fail;
                }
                status = parser_initializer_list(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid initializer list");
                        goto fail;
                }
                pt_node_child_add(expr, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) 
                        != PARSER_PUNCTUATOR(punc_right_ql_br)) {
                        status = parser_error(ps, "expected } symbol");
                        goto fail;
                }
                parser_pull_token(ps);

        } else {
                parser_put_token(ps, tok);
        }

        /* no type-name found so primary expr expected now */
        if (pt_node_child_empty(expr)) {
                status = parser_primary_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid initializer list");
                        goto fail;
                }
                pt_node_child_add(expr, parser_stack_pop(ps));
        }

        enum parser_symbol sym;
        while (postfix_second[sym = parser_fetch_symbol(ps)]) {
                if (sym == PARSER_PUNCTUATOR(punc_increment) 
                        || sym == PARSER_PUNCTUATOR(punc_decrement)) {
                        pt_node_child_add(expr, pt_node_create_leaf(
                                parser_pull_token(ps)));
                } else if (sym == PARSER_PUNCTUATOR(punc_dot) 
                        || sym == PARSER_PUNCTUATOR(punc_right_arrow)) {
                        pt_node_child_add(expr, pt_node_create_leaf(
                                parser_pull_token(ps)));                
                        
                        if (parser_fetch_symbol(ps) != psym_identifier) {
                                status = parser_error(ps, 
                                        "expected identifier");
                                goto fail;
                        }
                        pt_node_child_add(expr, pt_node_create_leaf(
                                parser_pull_token(ps)));                
                } else if (sym == PARSER_PUNCTUATOR(punc_left_sq_br)) {
                        parser_pull_token(ps);

                        if (!expr_first[parser_fetch_symbol(ps)]) {
                                status = parser_error(ps, "expected expression");
                                goto fail;
                        }

                        status = parser_expression(ps);
                        if (!status) {
                                status = parser_error(ps, "invalid expression");
                                goto fail;
                        }
                        assert(parser_stack_top(ps)->sym 
                                == psym_expression);
                        pt_node_child_add(expr, parser_stack_pop(ps));

                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_right_sq_br)) {
                                status = parser_error(ps, "expected ] symbol");
                                goto fail;
                        }
                        parser_pull_token(ps);
                } else {
                        parser_pull_token(ps);

                        if (arg_expr_lst[parser_fetch_symbol(ps)]) {
                                status = parser_argument_expression_list(ps);
                                if (!MC_SUCC(status)) {
                                        status = parser_error(ps, 
                                                "invalid argument expression list");
                                        goto fail;
                                }
                                pt_node_child_add(expr, parser_stack_pop(ps));
                        }

                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                                status = parser_error(ps, "expected ) symbol");
                                goto fail;

                        }
                        parser_pull_token(ps);
                }
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_unary_operator(struct parser *ps)
{
        struct pt_node *op = pt_node_create(psym_cast_expression);
        parser_stack_push(ps, op);
        pt_node_child_add(op, pt_node_create_leaf(parser_pull_token(ps)));
        return MC_OK;
}

/* note: parsed type-name could be in lookahead  */
static mc_status_t parser_unary_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t unary_expr_first[] = { UNARY_EXPRESSION };
        static const uint8_t cast_expr_first[] = { CAST_EXPRESSION };
        static const uint8_t type_name_first[] = { TYPE_NAME };
        static const uint8_t unary_op_first[] = { UNARY_OPERATOR };

        struct pt_node *expr = pt_node_create(psym_cast_expression);
        parser_stack_push(ps, expr);
        struct pt_node *u_expr = parser_lookahead_pull(ps, psym_unary_expression);

        if (u_expr != NULL && pt_node_child_count(u_expr) != 0) {
                parser_lookahead_swap(ps, expr);
                parser_lookahead_set_type(ps, psym_postfix_expression);
                status = parser_postfix_expression(ps);
                if (!status) {
                        parser_lookahead_restore(ps, NULL);
                        status = parser_error(ps, "invalid postfix expression");
                        goto fail;
                }
                assert(parser_stack_top(ps)->sym == psym_postfix_expression);
                pt_node_child_add(expr, parser_stack_pop(ps));
                return MC_OK;
        } else if (u_expr != NULL) {
                pt_node_destroy(u_expr);
        }

        enum parser_symbol sym = parser_fetch_symbol(ps);
        do {
                if (sym == PARSER_PUNCTUATOR(punc_increment) 
                        || sym == PARSER_PUNCTUATOR(punc_decrement)) {
                        pt_node_child_add(expr, 
                                pt_node_create_leaf(parser_pull_token(ps)));

                        if (!unary_expr_first[parser_fetch_symbol(ps)]) {
                                status = parser_error(ps, 
                                        "expected unary expression");
                                goto fail;
                        }
                } else if (sym == PARSER_KEYWORD(keyw_sizeof)) {
                        pt_node_child_add(expr, 
                                pt_node_create_leaf(parser_pull_token(ps)));

                        sym = parser_fetch_symbol(ps);
                        if (sym == PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                                parser_pull_token(ps);

                                if (!type_name_first[parser_fetch_symbol(ps)]) {
                                        status = parser_error(ps, 
                                                "expected type name");
                                        goto fail;
                                }
                                status = parser_type_name(ps);
                                if (!MC_SUCC(status)) {
                                        status = parser_error(ps, 
                                                "invalid type name");
                                        goto fail;
                                }
                                pt_node_child_add(expr, parser_stack_pop(ps));

                                if (parser_fetch_symbol(ps) != 
                                        PARSER_PUNCTUATOR( punc_right_rnd_br)) {
                                        status = parser_error(ps, 
                                                "expected ) symbol");       
                                }
                                parser_pull_token(ps);
                                break;
                        } else if (!unary_expr_first[sym]) {
                                status = parser_error(ps, 
                                        "expected unary expression");
                                goto fail;
                        }
                } else if (unary_op_first[sym]) {
                        status = parser_unary_operator(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, 
                                        "invalid unary operator");
                                goto fail;
                        }
                        pt_node_child_add(expr, parser_stack_pop(ps));

                        if (!cast_expr_first[parser_fetch_symbol(ps)]) {
                                status = parser_error(ps, 
                                        "expected cast expression");
                                goto fail;
                        }

                        status = parser_cast_expression(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, 
                                        "invalid cast expression");
                                goto fail;
                        }
                        pt_node_child_add(expr, parser_stack_pop(ps));
                        break;
                } else {
                        status = parser_postfix_expression(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps,
                                        "invalid postfix expression");
                                goto fail;
                        }
                        pt_node_child_add(expr, parser_stack_pop(ps));
                        break;
                }

        } while (unary_expr_first[sym = parser_fetch_symbol(ps)]);

        return MC_OK;
fail:
        parser_stack_restore(ps, expr);
        return status;
}

/* note: parsed unary expression OR type-name could be in 
 * lookahead, after processing in assignment expression 
 */
static mc_status_t parser_cast_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t cast_expr_first[] = { CAST_EXPRESSION };
        static const uint8_t type_name_first[] = { TYPE_NAME };
        struct pt_node *expr = parser_lookahead_pull(ps, psym_cast_expression);
        if (expr == NULL)
                expr = pt_node_create(psym_cast_expression);
        parser_stack_push(ps, expr);

        if (pt_node_child_type(expr, 0) == psym_unary_expression)
                return MC_OK;

        assert(pt_node_child_type(expr, 0) == psym_type_name);
        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (!cast_expr_first[sym])
                goto fail;

        /* we need to differ unary-expression in from, from ( type-name )
         * note, that one (type-name) could already be in expr */
        do {
                struct token *tok;
                if ((tok = parser_pull_token(ps))->type == 
                        PARSER_PUNCTUATOR(punc_left_rnd_br) 
                        && type_name_first[parser_fetch_symbol(ps)]) {

                        struct pt_node *old_lk = parser_lookahead_swap(ps, 
                                pt_node_create(psym_unary_expression));

                        status = parser_type_name(ps);
                        if (!MC_SUCC(status)) {
                                parser_lookahead_restore(ps, old_lk);
                                goto fail;
                        }

                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                                parser_lookahead_restore(ps, old_lk);
                                goto fail;
                        }
                        parser_pull_token(ps);
                        
                        if (parser_fetch_symbol(ps) 
                                == PARSER_PUNCTUATOR(punc_left_ql_br)) {
                                /* add type-name from stack */
                                parser_lookahead_add(ps, parser_stack_pop(ps));

                                status = parser_unary_expression(ps);
                                if (!MC_SUCC(status)) {
                                        parser_lookahead_restore(ps, old_lk);
                                        goto fail;
                                }       
                                assert(parser_stack_top(ps)->sym 
                                        == psym_unary_expression);

                                old_lk = parser_lookahead_swap(ps, old_lk);
                                assert(old_lk == NULL);
                        } else {
                                parser_lookahead_restore(ps, old_lk);
                        }

                        pt_node_child_add(expr, parser_stack_pop(ps));
                        continue;
                }
                parser_put_token(ps, tok);

                status = parser_unary_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));       
        } while (cast_expr_first[sym = parser_fetch_symbol(ps)]);

        if (pt_node_child_type(expr, pt_node_child_count(expr) - 1) 
                != psym_unary_expression)
                goto fail;

        return MC_OK;
fail:
        status = parser_error(ps, "expected unary expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_multiplicative_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t cast_expr_first[] = { CAST_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_multiplicative_expression);
        parser_stack_push(ps, expr);

        do {
                status = parser_cast_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (sym == PARSER_PUNCTUATOR(punc_mul) 
                        || sym == PARSER_PUNCTUATOR(punc_forward_slash)
                        || sym == PARSER_PUNCTUATOR(punc_percent)) {
                        struct pt_node *punc = pt_node_create_leaf(
                                parser_pull_token(ps));
                        pt_node_child_add(expr, punc);
                } else {
                        break;
                }
        } while (cast_expr_first[parser_fetch_symbol(ps)]);
        if (pt_node_child_count(expr) % 2 == 0)
                goto fail;

        return MC_OK;
fail:
        status = parser_error(ps, "expected cast expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_additive_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t add_expr_first[] = { MULTIPLICATIVE_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_additive_expression);
        parser_stack_push(ps, expr);

        do {
                status = parser_multiplicative_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (sym == PARSER_PUNCTUATOR(punc_add) 
                        || sym == PARSER_PUNCTUATOR(punc_sub)) {
                        struct pt_node *punc = pt_node_create_leaf(
                                parser_pull_token(ps));
                        pt_node_child_add(expr, punc);
                } else {
                        break;
                }
        } while (add_expr_first[parser_fetch_symbol(ps)]);
        if (pt_node_child_count(expr) % 2 == 0)
                goto fail;

        return MC_OK;
fail:
        status = parser_error(ps, "expected multiplicative expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_shift_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t add_expr_first[] = { ADDITIVE_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_shift_expression);
        parser_stack_push(ps, expr);

        do {
                status = parser_additive_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (sym == PARSER_PUNCTUATOR(punc_shl) 
                        || sym == PARSER_PUNCTUATOR(punc_shr)) {
                        struct pt_node *punc = pt_node_create_leaf(
                                parser_pull_token(ps));
                        pt_node_child_add(expr, punc);
                } else {
                        break;
                }
        } while (add_expr_first[parser_fetch_symbol(ps)]);
        if (pt_node_child_count(expr) % 2 == 0)
                goto fail;

        return MC_OK;
fail:
        status = parser_error(ps, "expected additive expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_relational_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t shift_first[] = { SHIFT_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_relational_expression);
        parser_stack_push(ps, expr);

        do {
                status = parser_shift_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (sym == PARSER_PUNCTUATOR(punc_less) 
                        || sym == PARSER_PUNCTUATOR(punc_greater)
                        || sym == PARSER_PUNCTUATOR(punc_less_eq)
                        || sym == PARSER_PUNCTUATOR(punc_greater_eq)) {
                        struct pt_node *punc = pt_node_create_leaf(
                                parser_pull_token(ps));
                        pt_node_child_add(expr, punc);
                } else {
                        break;
                }
        } while (shift_first[parser_fetch_symbol(ps)]);
        if (pt_node_child_count(expr) % 2 == 0)
                goto fail;

        return MC_OK;
fail:
        status = parser_error(ps, "expected shift expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_equality_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t rel_expr[] = { RELATIONAL_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_equality_expression);
        parser_stack_push(ps, expr);

        do {
                status = parser_relational_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (sym == PARSER_PUNCTUATOR(punc_equal) 
                        || sym == PARSER_PUNCTUATOR(punc_not_equal)) {
                        struct pt_node *punc = pt_node_create_leaf(
                                parser_pull_token(ps));
                        pt_node_child_add(expr, punc);
                } else {
                        break;
                }
        } while (rel_expr[parser_fetch_symbol(ps)]);
        if (pt_node_child_count(expr) % 2 == 0)
                goto fail;

        return MC_OK;
fail:
        status = parser_error(ps, "expected relational expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_and_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t equ_expr[] = { EQUALITY_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_and_expression);
        parser_stack_push(ps, expr);

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (!equ_expr[sym])
                        goto fail;
                status = parser_equality_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_bit_and) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        status = parser_error(ps, "expected equality expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_exclusive_or_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t and_expr[] = { AND_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_exclusive_or_expression);
        parser_stack_push(ps, expr);

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (!and_expr[sym])
                        goto fail;
                status = parser_and_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_xor) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        status = parser_error(ps, "expected and expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_inclusive_or_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t exc_expr_first[] = { EXCLUSIVE_OR_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_inclusive_or_expression);
        parser_stack_push(ps, expr);

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (!exc_expr_first[sym])
                        goto fail;
                status = parser_exclusive_or_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_bar) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        status = parser_error(ps, "expected inclusive or expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_logical_and_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t inc_expr_first[] = { INCLUSIVE_OR_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_logical_and_expression);
        parser_stack_push(ps, expr);

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (!inc_expr_first[sym])
                        goto fail;
                status = parser_inclusive_or_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_logical_and) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        status = parser_error(ps, "expected inclusive or expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_logical_or_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t and_expr_first[] = { LOGICAL_AND_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_logical_or_expression);
        parser_stack_push(ps, expr);

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (!and_expr_first[sym])
                        goto fail;
                status = parser_logical_and_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_logical_or) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        status = parser_error(ps, "expected logical and expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_assignment_operator(struct parser *ps)
{
        struct pt_node *assign_op = pt_node_create(psym_assignment_operator);
        parser_stack_push(ps, assign_op);
        struct pt_node *punc = pt_node_create_leaf(parser_pull_token(ps));
        pt_node_child_add(assign_op, punc);
        return MC_OK;
}

static mc_status_t parser_assignment_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t type_name_first[] = { TYPE_NAME };
        static const uint8_t assig_op_first[] = { ASSIGNMENT_OPERATOR };
        static const uint8_t assig_expr_first[] = { ASSIGNMENT_EXPRESSION };
        struct pt_node *assig_expr = pt_node_create(psym_assignment_expression);
        parser_stack_push(ps, assig_expr);
        _Bool unary_expr = true; /* is unary expr in front ? */

        /* We have common unary expression in conditional expression, and
         * second one possible production. We differ them by parsing 
         * ( type-name ). And then checking for '{' after and parse
         * unary expression if '{' met, and conditional-expression otherwise 
         */
        struct pt_node *old_lk = parser_lookahead_swap(ps, 
                pt_node_create(psym_invalid));
        
        if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                /* to fetch first symbol of expr after '(' */
                struct token *rnd_br = parser_pull_token(ps);
                enum parser_symbol sym = parser_fetch_symbol(ps);
                parser_put_token(ps, rnd_br);

                /* type name from postfix-expr or cast expr OR from expr */
                if (type_name_first[sym]) {
                        status = parser_type_name(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, "invalid type name");
                                goto fail;
                        }
                        /* add type name to lookahead symbol */
                        parser_lookahead_add(ps, parser_stack_pop(ps));

                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                                status = parser_error(ps, "expected )");
                                goto fail;
                        }
                        parser_pull_token(ps);

                        if (parser_fetch_symbol(ps) 
                                != PARSER_PUNCTUATOR(punc_left_ql_br)) {
                                unary_expr = false;
                        }
                } 
        }

        if (unary_expr) {
                parser_lookahead_set_type(ps, psym_unary_expression);
                status = parser_unary_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid unary expression");
                        goto fail;
                }
                assert(parser_stack_top(ps)->sym == psym_unary_expression);
        }
        if (unary_expr && assig_op_first[parser_fetch_symbol(ps)]) {
                pt_node_child_add(assig_expr, parser_stack_pop(ps));

                status = parser_assignment_operator(ps);
                assert(MC_SUCC(status));
                pt_node_child_add(assig_expr, parser_stack_pop(ps));

                if (!assig_expr_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected assignment expression");
                        goto fail;
                }
                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid assignment expression");
                        goto fail;
                }
                pt_node_child_add(assig_expr, parser_stack_pop(ps));
        } else {
                if (unary_expr) {
                        struct pt_node *consumed = parser_lookahead_swap(ps, 
                                pt_node_create(psym_invalid));
                        assert(consumed == NULL);
                        parser_lookahead_add(ps, parser_stack_pop(ps));
                }
                parser_lookahead_set_type(ps, psym_cast_expression);
                status = parser_conditional_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid unary expression");
                        goto fail;
                }
                pt_node_child_add(assig_expr, parser_stack_pop(ps));
        }

        /* lookahed symbol restored and consumed */
        old_lk = parser_lookahead_swap(ps, old_lk);
        assert(old_lk == NULL);
        
        return MC_OK;
fail:
        parser_lookahead_restore(ps, old_lk);
        parser_stack_restore(ps, assig_expr);
        return status;
}

static mc_status_t parser_expression(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t assig_expr[] = { ASSIGNMENT_EXPRESSION };
        struct pt_node *expr = pt_node_create(psym_expression);
        parser_stack_push(ps, expr);

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (!assig_expr[sym])
                        goto fail;
                status = parser_assignment_expression(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(expr, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        status = parser_error(ps, "expected assignment expr");
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_conditional_expression(struct parser *ps)
{
        static const uint8_t expr_first[] = { EXPRESSION };
        static const uint8_t cond_expr_first[] = { CONDITIONAL_EXPRESSION };
        struct pt_node *cond_expr = pt_node_create(psym_constant_expression);
        parser_stack_push(ps, cond_expr);
        
        mc_status_t status;
        do {
                status = parser_logical_or_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid logical OR expr");
                        goto fail;
                }
                pt_node_child_add(cond_expr, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_question))
                        break;
                parser_pull_token(ps);

                if (!expr_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected expression");
                        goto fail;
                }
                status = parser_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid expression");
                        goto fail;
                }
                pt_node_child_add(cond_expr, parser_stack_pop(ps));

                if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_colon))
                        return MC_OK;
                parser_pull_token(ps);

                if (!cond_expr_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected conditional expr");
                        goto fail;
                }
        } while (1);

        return MC_OK;
fail:
        parser_stack_restore(ps, cond_expr);
        return status;
}

static mc_status_t parser_constant_expression(struct parser *ps)
{
        struct pt_node *const_expr = pt_node_create(psym_constant_expression);
        parser_stack_push(ps, const_expr);
        mc_status_t status = parser_conditional_expression(ps);
        if (!MC_SUCC(status)) {
                parser_stack_restore(ps, const_expr);
                return parser_error(ps, "invalid conditional expr");
        }
        pt_node_child_add(const_expr, parser_stack_pop(ps));
        return MC_OK;
}

static mc_status_t parser_struct_declarator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t decl_first[] = { DECLARATOR };
        static const uint8_t constex_first[] = { DECLARATOR };
        struct pt_node *st_decl = pt_node_create(psym_struct_declarator);
        parser_stack_push(ps, st_decl);

        if (decl_first[parser_fetch_symbol(ps)]) {
                status = parser_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid declarator");
                        goto fail;
                }
                pt_node_child_add(st_decl, parser_stack_pop(ps));
        }

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (sym == PARSER_PUNCTUATOR(punc_colon)) {
                parser_pull_token(ps);
                if (!constex_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected const expr");
                        goto fail;
                }
                status = parser_constant_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid const expr");
                        goto fail;
                }
                pt_node_child_add(st_decl, parser_stack_pop(ps));
        } else if (pt_node_child_empty(st_decl)) {
                status = parser_error(ps, 
                        "after missing declarator(opt) expected :");
                goto fail;
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, st_decl);
        return status;
}
        
static mc_status_t parser_struct_declarator_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t st_decl[] = { STRUCT_DECLARATOR };
        struct pt_node *st_decl_lst = pt_node_create(
                psym_struct_declarator_list);
        parser_stack_push(ps, st_decl_lst);

        do {
                enum parser_symbol sym = parser_fetch_symbol(ps);
                if (!st_decl[sym])
                        goto fail;
                status = parser_struct_declarator(ps);
                if (!MC_SUCC(status))
                        goto fail;
                pt_node_child_add(st_decl_lst, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        status = parser_error(ps, "expected struct declarator");
        parser_stack_restore(ps, st_decl_lst);
        return status;
}

static mc_status_t parser_specifier_qualifier_list(struct parser *ps)
{
        static const uint8_t type_spec[] = { TYPE_SPECIFIER };
        static const uint8_t type_qual[] = { TYPE_QUALIFIER };
        mc_status_t status = MC_OK;
        struct pt_node *spec_ql_l = pt_node_create(psym_struct_declaration);
        parser_stack_push(ps, spec_ql_l);
        enum parser_symbol sym;
        while ((sym = parser_fetch_symbol(ps))) {
                if (type_spec[sym]) {
                        if (sym == psym_identifier && parser_get_symbol(ps, 
                                parser_fetch_token(ps)) != psym_typedef_name)
                                break;
                        status = parser_type_specifier(ps);
                } else if (type_qual[sym]) {
                        status = parser_type_qualifier(ps);
                } else {
                        break;
                }
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "expected type specifier or qualifier");
                        goto fail;
                }
                pt_node_child_add(spec_ql_l, parser_stack_pop(ps));
        }
        assert(!pt_node_child_empty(spec_ql_l));

        return MC_OK;
fail:
        parser_stack_restore(ps, spec_ql_l);
        return status;
}

static mc_status_t parser_struct_declaration(struct parser *ps)
{
        static const uint8_t st_decl_l[] = { STRUCT_DECLARATION_LIST };
        struct pt_node *st_decl = pt_node_create(psym_struct_declaration);
        parser_stack_push(ps, st_decl);
        mc_status_t status = parser_specifier_qualifier_list(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "expected specifier/qualifier");
                goto fail;
        }
        pt_node_child_add(st_decl, parser_stack_pop(ps));
        if (!st_decl_l[parser_fetch_symbol(ps)]
                || !MC_SUCC(status = parser_struct_declarator_list(ps))) {
                status = parser_error(ps, "expected struct declarator");
                goto fail;
        }
        pt_node_child_add(st_decl, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_semicolon)) {
                status = parser_error(ps, 
                        "expected ; at the end of struct declaration");
                goto fail;
        }
        parser_pull_token(ps);

        return MC_OK;
fail:
        parser_stack_restore(ps, st_decl);
        return status;
}

static mc_status_t parser_struct_declaration_list(struct parser *ps)
{
        mc_status_t status;
        enum parser_symbol sym;
        static const uint8_t st_decl[] = { STRUCT_DECLARATION };

        struct pt_node *st_decl_lst = pt_node_create(
                psym_struct_declaration_list);
        parser_stack_push(ps, st_decl_lst);

        while ((sym = parser_fetch_symbol(ps)) && st_decl[sym]) {
                status = parser_struct_declaration(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid struct declaration");
                        goto fail;
                }
                pt_node_child_add(st_decl_lst, parser_stack_pop(ps));
        }
        return MC_OK;
fail:
        parser_stack_restore(ps, st_decl_lst);
        return status;
}


static mc_status_t parser_struct_or_union_specifier(struct parser *ps)
{
        struct pt_node *id_val = NULL;
        struct pt_node *spec = pt_node_create(psym_struct_or_union_specifier);
        parser_stack_push(ps, spec);
        mc_status_t status = parser_struct_or_union(ps);
        assert(MC_SUCC(status));
        pt_node_child_add(spec, parser_stack_pop(ps));

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (sym == psym_identifier) {
                id_val = pt_node_create_leaf(parser_pull_token(ps));
                pt_node_child_add(spec, id_val);
        }
                
        sym = parser_fetch_symbol(ps);
        static const uint8_t st_decl_l[] = { STRUCT_DECLARATION_LIST };
        if (sym == PARSER_PUNCTUATOR(punc_left_ql_br)) {

                parser_pull_token(ps);

                sym = parser_fetch_symbol(ps);
                if (!st_decl_l[sym]) {
                        status = parser_error(ps, 
                        "expected struct declaration list inside {}");
                        goto fail;
                }
                status = parser_struct_declaration_list(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                        "invalid struct declaration list");
                        goto fail;
                }

                sym = parser_fetch_symbol(ps);
                if (sym != PARSER_PUNCTUATOR(punc_right_ql_br)) {
                        status = parser_error(ps, 
                        "missing closing } bracer");
                        goto fail;
                }
                parser_pull_token(ps);
        } else if (id_val == NULL) {
                status = parser_error(ps, 
                        "no identifier, so struct declaration list expected");
                goto fail;
        }

        return status;
fail:
        parser_stack_restore(ps, spec);
        return MC_FAIL;
}

static mc_status_t parser_function_specifier(struct parser *ps)
{
        struct pt_node *func_spec = pt_node_create(psym_function_specifier);
        parser_stack_push(ps, func_spec);
        /* inline keyword */
        pt_node_child_add(func_spec, pt_node_create_leaf(
                parser_pull_token(ps)));
        return MC_OK;
}

/* next term: first in declaration-specifiers */
static mc_status_t parser_declaration_specifiers(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t st_class_spec[] = { STORAGE_CLASS_SPECIFIER };
        static const uint8_t type_spec[] = { TYPE_SPECIFIER };
        static const uint8_t type_qual[] = { TYPE_QUALIFIER };
        static const uint8_t func_spec[] = { FUNCTION_SPECIFIER };
        struct pt_node *decl_spec = pt_node_create(psym_declaration_specifiers);
        parser_stack_push(ps, decl_spec);

        enum parser_symbol sym;
        while ((sym = parser_fetch_symbol(ps))) {
                if (st_class_spec[sym]) {
                        status = parser_storage_class_specifier(ps);
                } else if (type_spec[sym]) {
                        if (sym == psym_identifier && parser_get_symbol(ps, 
                                parser_fetch_token(ps)) != psym_typedef_name)
                                break;
                        status = parser_type_specifier(ps);
                } else if (type_qual[sym]) {
                        status = parser_type_qualifier(ps);
                } else if (func_spec[sym]) {
                        status = parser_function_specifier(ps);
                } else {
                        break;
                }
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid specifier");
                        goto fail;
                }
                pt_node_child_add(decl_spec, parser_stack_pop(ps));
        }
        return MC_OK;
fail:
        parser_stack_restore(ps, decl_spec);
        return status;
}

static mc_status_t parser_declaration_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t decl_first[] = { DECLARATION };
        struct pt_node *decl = pt_node_create(psym_declaration_list);
        parser_stack_push(ps, decl);

        enum parser_symbol sym = parser_fetch_symbol(ps);
        do {
                status = parser_declaration(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid declaration");
                        goto fail;
                }
                pt_node_child_add(decl, parser_stack_pop(ps));
        } while (decl_first[sym = parser_fetch_symbol(ps)]);

        return MC_OK;
fail:
        parser_stack_restore(ps, decl);
        return status;
}

static mc_status_t parser_labeled_statement(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t const_first[] = { CONSTANT_EXPRESSION };
        static const uint8_t stmt_first[] = { STATEMENT };
        struct pt_node *labeled = pt_node_create(psym_labeled_statement);
        parser_stack_push(ps, labeled);

        enum parser_symbol sym = parser_fetch_symbol(ps);
        /* add identifier, case or default keywords */
        pt_node_child_add(labeled, pt_node_create_leaf(
                parser_pull_token(ps)));

        if (sym == PARSER_KEYWORD(keyw_case)) {

                if (!const_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, 
                                "expected constant expression");
                        goto fail;
                }

                status = parser_constant_expression(ps); 
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, 
                                "invalid constant expression");
                        goto fail;
                }
                pt_node_child_add(labeled, parser_stack_pop(ps));
        }

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_colon)) {
                status = parser_error(ps, "expected : symbol");
                goto fail;
        }
        parser_pull_token(ps);

        if (!stmt_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected statement");
                goto fail;
        }
        status = parser_statement(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid statement");
                goto fail;
        }
        pt_node_child_add(labeled, parser_stack_pop(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, labeled);
        return status;
}

static mc_status_t parser_expression_statement(struct parser *ps)
{
        mc_status_t status;
        struct pt_node *expr = pt_node_create(psym_expression_statement);
        parser_stack_push(ps, expr);

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_semicolon)) {
                status = parser_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid expression");
                        goto fail;
                }
                pt_node_child_add(expr, parser_stack_pop(ps));
        }

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_semicolon)) {
                status = parser_error(ps, "expected ; symbol");
                goto fail;
        }
        parser_pull_token(ps);

        return MC_OK;
fail:
        parser_stack_restore(ps, expr);
        return status;
}

static mc_status_t parser_selection_statement(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t expr_first[] = { EXPRESSION };
        static const uint8_t stmt_first[] = { STATEMENT };
        struct pt_node *selection = pt_node_create(psym_selection_statement);
        parser_stack_push(ps, selection);

        /* add if or switch keyword */
        pt_node_child_add(selection, pt_node_create_leaf(
                parser_pull_token(ps)));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                status = parser_error(ps, "expected ( symbol");
                goto fail;
        }
        /* pull ( symbol */
        parser_pull_token(ps);

        if (!expr_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected expression");
                goto fail;
        }

        status = parser_expression(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid expression");
                goto fail;
        }
        pt_node_child_add(selection, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                status = parser_error(ps, "expected ) symbol");
                goto fail;
        }
        /* pull ) symbol */
        parser_pull_token(ps);

        if (!stmt_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected statement");
                goto fail;
        }

        status = parser_statement(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid statement");
                goto fail;
        }
        pt_node_child_add(selection, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) == PARSER_KEYWORD(keyw_else)) {
                parser_pull_token(ps);

                if (!stmt_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected statement");
                        goto fail;
                }

                status = parser_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid statement");
                        goto fail;
                }
                pt_node_child_add(selection, parser_stack_pop(ps));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, selection);
        return status;
}

static mc_status_t parser_iteration_while(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t expr_first[] = { EXPRESSION };
        static const uint8_t stmt_first[] = { STATEMENT };
        struct pt_node *iter = pt_node_create(psym_iteration_statement);
        parser_stack_push(ps, iter);

        /* add while keyword */
        pt_node_child_add(iter, pt_node_create_leaf(
                parser_pull_token(ps)));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                status = parser_error(ps, "expected ( symbol");
                goto fail;
        }
        parser_pull_token(ps);

        if (!expr_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected expression");
                goto fail;
        }
        status = parser_expression(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid expression");
                goto fail;
        }
        pt_node_child_add(iter, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                status = parser_error(ps, "expected ) symbol");
                goto fail;
        }
        parser_pull_token(ps);

        if (!stmt_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected statement");
                goto fail;
        }

        status = parser_statement(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid statement");
                goto fail;
        }
        pt_node_child_add(iter, parser_stack_pop(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, iter);
        return status;
}

static mc_status_t parser_iteration_for(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t expr_first[] = { EXPRESSION };
        static const uint8_t decl_first[] = { DECLARATION };
        static const uint8_t stmt_first[] = { STATEMENT };
        struct pt_node *iter = pt_node_create(psym_iteration_statement);
        parser_stack_push(ps, iter);

        /* add for keyword */
        pt_node_child_add(iter, pt_node_create_leaf(
                parser_pull_token(ps)));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                status = parser_error(ps, "expected ( symbol");
                goto fail;
        }
        parser_pull_token(ps);

        _Bool is_declaration = false;
        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (sym == psym_identifier && parser_get_symbol(ps, 
                parser_fetch_token(ps)) == psym_typedef_name)
                is_declaration = true;
        
        if ((decl_first[sym] && sym != psym_identifier) || is_declaration) {
                status = parser_declaration(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid declaration");
                        goto fail;
                }
                pt_node_child_add(iter, parser_stack_pop(ps));
                sym = parser_fetch_symbol(ps);
        }

        if (expr_first[sym]) {
                status = parser_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid expression");
                        goto fail;
                }
                pt_node_child_add(iter, parser_stack_pop(ps));
                sym = parser_fetch_symbol(ps);
        }

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_semicolon)) {
                status = parser_error(ps, "expected ; in for (...; ");
                goto fail;
        }
        parser_pull_token(ps);

        if (expr_first[sym = parser_fetch_symbol(ps)]) {
                status = parser_expression(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "expected expression");
                        goto fail;
                }
                pt_node_child_add(iter, parser_stack_pop(ps));
                sym = parser_fetch_symbol(ps);
        }

        if (sym == PARSER_PUNCTUATOR(punc_semicolon)) {
                parser_pull_token(ps);
                if (expr_first[sym = parser_fetch_symbol(ps)]) {
                        status = parser_expression(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, "invalid expression");
                                goto fail;
                        }
                        pt_node_child_add(iter, parser_stack_pop(ps));
                        sym = parser_fetch_symbol(ps);
                }       
        }

        if (sym != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                status = parser_error(ps, "expected ) symbol");
                goto fail;
        }
        parser_pull_token(ps);

        if (stmt_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected statement");
                goto fail;
        }
        status = parser_statement(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid statement");
                goto fail;
        }
        pt_node_child_add(iter, parser_stack_pop(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, iter);
        return status;
}

static mc_status_t parser_iteration_do(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t stmt_first[] = { STATEMENT };
        static const uint8_t expr_first[] = { EXPRESSION };
        struct pt_node *iter = pt_node_create(psym_iteration_statement);
        parser_stack_push(ps, iter);

        /* add do keyword */
        pt_node_child_add(iter, pt_node_create_leaf(
                parser_pull_token(ps)));

        if (stmt_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected statement");
                goto fail;
        }

        status = parser_statement(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid statement");
                goto fail;
        }
        pt_node_child_add(iter, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) != PARSER_KEYWORD(keyw_while)) {
                status = parser_error(ps, "expected while after stmt");
                goto fail;
        }
        parser_pull_token(ps);

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_left_rnd_br)) {
                status = parser_error(ps, "expected ( symbol");
                goto fail;
        }
        parser_pull_token(ps);

        if (!expr_first[parser_fetch_symbol(ps)]) {
                status = parser_error(ps, "expected expression");
                goto fail;
        }
        status = parser_expression(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid expression");
                goto fail;
        }
        pt_node_child_add(iter, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_right_rnd_br)) {
                status = parser_error(ps, "expected ) symbol");
                goto fail;
        }
        parser_pull_token(ps);

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_semicolon)) {
                status = parser_error(ps, "expected ; symbol");
                goto fail;
        }
        parser_pull_token(ps);

        return MC_OK;
fail:
        parser_stack_restore(ps, iter);
        return status;
}

static mc_status_t parser_iteration_statement(struct parser *ps)
{
        mc_status_t status;
        enum parser_symbol sym = parser_fetch_symbol(ps);

        if (sym == PARSER_KEYWORD(keyw_while))
                status = parser_iteration_while(ps);
        else if (sym == PARSER_KEYWORD(keyw_for))
                status = parser_iteration_for(ps);
        else
                status = parser_iteration_do(ps);

        return status;
}

static mc_status_t parser_jump_statement(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t expr_first[] = { EXPRESSION };
        struct pt_node *jump = pt_node_create(psym_jump_statement);
        parser_stack_push(ps, jump);
        enum parser_symbol sym = parser_fetch_symbol(ps);
        /* add goto, continue, break or return keyword */
        pt_node_child_add(jump, pt_node_create_leaf(parser_pull_token(ps)));

        /* cast to avoid false-alarm warning here */
        switch ((int)sym) {
                case PARSER_KEYWORD(keyw_goto):
                {
                        if (parser_fetch_symbol(ps) != psym_identifier) {
                                status = parser_error(ps, "expected identifier");
                                goto fail;
                        }
                        pt_node_child_add(jump, 
                                pt_node_create_leaf(parser_pull_token(ps)));
                }
                case PARSER_KEYWORD(keyw_continue):
                case PARSER_KEYWORD(keyw_break):
                {
                        break;
                }
                case PARSER_KEYWORD(keyw_return):
                {
                        if (!expr_first[parser_fetch_symbol(ps)])
                                break;

                        status = parser_expression(ps);
                        if (!MC_SUCC(status)) {
                                status = parser_error(ps, "invalid expression");
                                goto fail;
                        }
                        pt_node_child_add(jump, parser_stack_pop(ps));
                        break;
                }
                default:
                {
                        MC_LOG(MC_CRIT, "unexpected jump keyword");
                        break;
                }
        }
        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_semicolon)) {
                status = parser_error(ps, "expected identifier");
                goto fail;
        }
        parser_pull_token(ps);
        return MC_OK;
fail:
        parser_stack_restore(ps, jump);
        return status;
}

static mc_status_t parser_statement(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t comp_first[] = { COMPOUND_STATEMENT };
        static const uint8_t expr_first[] = { EXPRESSION_STATEMENT };
        static const uint8_t select_first[] = { SELECTION_STATEMENT };
        static const uint8_t iter_first[] = { ITERATION_STATEMENT };
        static const uint8_t jump_first[] = { JUMP_STATEMENT };
        struct pt_node *stmt = pt_node_create(psym_statement);
        parser_stack_push(ps, stmt);

        /* we should differ identifier in labeled statement 
         * from one in expression statement by a colon sym */
        _Bool is_labeled_stmt = false;

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (sym == psym_identifier) {
                struct token *id_tok = parser_pull_token(ps);
                if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_colon))
                        is_labeled_stmt = true;
                parser_put_token(ps, id_tok);
        }

        if (comp_first[sym]) {
                status = parser_compound_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid compound statement");
                        goto fail;
                }
                pt_node_child_add(stmt , parser_stack_pop(ps));
        } else if (select_first[sym]) {
                status = parser_selection_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid selection statement");
                        goto fail;
                }
                pt_node_child_add(stmt , parser_stack_pop(ps));
        } else if (iter_first[sym]) {
                status = parser_iteration_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid iteration statement");
                        goto fail;
                }
                pt_node_child_add(stmt , parser_stack_pop(ps));
        } else if (jump_first[sym]) {
                status = parser_jump_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid jump statement");
                        goto fail;
                }
                pt_node_child_add(stmt , parser_stack_pop(ps));
        } else if (expr_first[sym] && !is_labeled_stmt) {
                status = parser_expression_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid expression statement");
                        goto fail;
                }
                pt_node_child_add(stmt , parser_stack_pop(ps));
        } else {
                status = parser_labeled_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid labeled statement");
                        goto fail;
                }
                pt_node_child_add(stmt , parser_stack_pop(ps));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, stmt);
        return status;
}

static mc_status_t parser_block_item(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t stmt_first[] = { STATEMENT };
        struct pt_node *blk_itm = pt_node_create(psym_block_item);
        parser_stack_push(ps, blk_itm);

        _Bool is_declaration = false;
        enum parser_symbol sym = parser_fetch_symbol(ps);
        /* under declaration this could be typedef name OR identifier 
         * from expression from statement */
        if (sym == psym_identifier && parser_get_symbol(ps, 
                parser_fetch_token(ps)) == psym_typedef_name)
                is_declaration = true;

        if (stmt_first[parser_fetch_symbol(ps)] && !is_declaration) {
                status = parser_statement(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid statement");
                        goto fail;
                }
                pt_node_child_add(blk_itm , parser_stack_pop(ps));
        } else {
                status = parser_declaration(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid declaration");
                        goto fail;
                }
                pt_node_child_add(blk_itm , parser_stack_pop(ps));
        }
        return MC_OK;
fail:
        parser_stack_restore(ps, blk_itm);
        return status;
}

static mc_status_t parser_block_item_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t stmt_first[] = { STATEMENT };
        static const uint8_t decl_first[] = { DECLARATION };
        struct pt_node *blk_itm_lst = pt_node_create(psym_block_item_list);
        parser_stack_push(ps, blk_itm_lst);

        enum parser_symbol sym;
        do {
                status = parser_block_item(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid block item");
                        goto fail;
                }
                pt_node_child_add(blk_itm_lst , parser_stack_pop(ps));
        } while (stmt_first[sym = parser_fetch_symbol(ps)] || decl_first[sym]);

        return MC_OK;
fail:
        parser_stack_restore(ps, blk_itm_lst);
        return status;
}

static mc_status_t parser_compound_statement(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t stmt_first[] = { STATEMENT };
        static const uint8_t decl_first[] = { DECLARATION };
        struct pt_node *comp_stmt = pt_node_create(psym_compound_statement);
        parser_stack_push(ps, comp_stmt);
        /* pull { symbol */
        parser_pull_token(ps);

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (stmt_first[sym] || decl_first[sym]) {
                status = parser_block_item_list(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid block item list");
                        goto fail;
                }
                pt_node_child_add(comp_stmt, parser_stack_pop(ps));
                sym = parser_fetch_symbol(ps);
        }
        if (sym != PARSER_PUNCTUATOR(punc_right_ql_br)) {
                status = parser_error(ps, "expected } symbol");
                goto fail;
        }
        parser_pull_token(ps);

        return MC_OK;
fail:
        parser_stack_restore(ps, comp_stmt);
        return status;
}

/* on stack: declarations-specifiers and declarator */
static mc_status_t parser_function_definition(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t decl_list[] = { DECLARATION_LIST };
        static const uint8_t comp_stmt[] = { COMPOUND_STATEMENT };
        struct pt_node *func_def = parser_lookahead_pull(ps, 
                psym_function_definition);
        /* currently we have only this func usage with lookahead */
        assert(func_def != NULL);
        parser_stack_push(ps, func_def);

        enum parser_symbol sym = parser_fetch_symbol(ps);
        if (decl_list[sym]) {
                status = parser_declaration_list(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid declaration list");
                        goto fail;
                }
                pt_node_child_add(func_def, parser_stack_pop(ps));
                sym = parser_fetch_symbol(ps);
        }

        if (!comp_stmt[sym]) {
                status = parser_error(ps, "expected compound statement");
                goto fail;
        }
        status = parser_compound_statement(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid compound statement");
                goto fail;
        }
        pt_node_child_add(func_def, parser_stack_pop(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, func_def);
        return status;
}

/* in lookahead: ?declarator */
static mc_status_t parser_init_declarator(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t init_first[] = { INITIALIZER };
        struct pt_node *decl = parser_lookahead_pull(ps, psym_init_declarator);
        if (decl == NULL)
                decl = pt_node_create(psym_init_declarator);
        parser_stack_push(ps, decl);

        if (pt_node_child_empty(decl)) {
                status = parser_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid declarator");
                        goto fail;
                }
                pt_node_child_add(decl, parser_stack_pop(ps));
        }

        if (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_assign)) {
                parser_pull_token(ps);

                if (!init_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected initializer");
                        goto fail;
                }

                status = parser_initializer(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid initializer");
                        goto fail;
                }
                pt_node_child_add(decl, parser_stack_pop(ps));
        }

        return MC_OK;
fail:
        parser_stack_restore(ps, decl);
        return status;
}

static mc_status_t parser_init_declarator_list(struct parser *ps)
{
        mc_status_t status;
        static const uint8_t decl_first[] = { DECLARATOR };
        struct pt_node *decl_lst = pt_node_create(psym_declaration);
        parser_stack_push(ps, decl_lst);

        status = parser_init_declarator(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "invalid init declarator");
                goto fail;
        }
        pt_node_child_add(decl_lst, parser_stack_pop(ps));

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_comma))
                return MC_OK;
        parser_pull_token(ps);

        do {
                if (!decl_first[parser_fetch_symbol(ps)]) {
                        status = parser_error(ps, "expected init declarator");
                        goto fail;
                }

                status = parser_init_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid init declarator");
                        goto fail;
                }
                pt_node_child_add(decl_lst, parser_stack_pop(ps));
        } while (parser_fetch_symbol(ps) == PARSER_PUNCTUATOR(punc_comma) 
                && parser_pull_token(ps));

        return MC_OK;
fail:
        parser_stack_restore(ps, decl_lst);
        return status;
}

/* in lookahead: declaration-specifiers and ?declarator */
static mc_status_t parser_declaration(struct parser *ps)
{
        mc_status_t status;
        struct pt_node *decl = parser_lookahead_pull(ps, psym_declaration);
        if (decl == NULL)
                decl = pt_node_create(psym_declaration);
        parser_stack_push(ps, decl);

        /* declarator from init-declarator present in lookahead */
        if (!pt_node_child_empty(decl) && pt_node_child_last(decl)->sym 
                == psym_declarator) {
                struct pt_node *declarator = pt_node_child_remove(decl);
                struct pt_node *old_lk = parser_lookahead_swap(ps, 
                        pt_node_create(psym_init_declarator));
                parser_lookahead_add(ps, declarator);

                status = parser_init_declarator_list(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "invalid init-declarator list");
                        parser_lookahead_restore(ps, old_lk);
                        goto fail;
                }
                pt_node_child_add(decl, parser_stack_pop(ps));
                old_lk = parser_lookahead_swap(ps, old_lk);
                assert(old_lk == NULL);
        }

        if (parser_fetch_symbol(ps) != PARSER_PUNCTUATOR(punc_semicolon)) {
                status = parser_error(ps, "expected ; symbol");
                goto fail;
        }
        parser_pull_token(ps);

        return MC_OK;
fail:
        parser_stack_restore(ps, decl);
        return status;
}

/* next term: first in trsnslation-unit */
static mc_status_t parser_external_declaration(struct parser *ps)
{
        static const uint8_t decl_first[] = { DECLARATOR };
        static const uint8_t decltion_second[] = { DECLARATION_SECOND };
        static const uint8_t decl_list_first[] = { DECLARATION_LIST };
        _Bool declarator = false;

        struct pt_node *ext_decl = pt_node_create(psym_external_declaration);
        parser_stack_push(ps, ext_decl);

        /* expext function-definition OR declaration, so we`ll lookahead 
         * for: declaration specifiers, declarator and first of decl list
         * OR  declaration specifiers declarator and second in init-decl-list
         * after declarator*/
        struct pt_node *old_lk = parser_lookahead_swap(ps, 
                pt_node_create(psym_invalid));

        /* first common lookahead symbol */
        mc_status_t status = parser_declaration_specifiers(ps);
        if (!MC_SUCC(status)) {
                status = parser_error(ps, "expected declaration-specifiers");
                goto fail;
        }
        assert(parser_stack_top(ps)->sym == psym_declaration_specifiers);
        /* add decl spec to lookahead symbol */
        parser_lookahead_add(ps, parser_stack_pop(ps));

        enum parser_symbol sym = parser_fetch_symbol(ps);
        /* declarator from function definition OR init declarator list */
        if (decl_first[sym]) {
                status = parser_declarator(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "expected declarator");
                        goto fail;
                }
                assert(parser_stack_top(ps)->sym == psym_declarator);

                /* add declarator to lookahead symbol */
                parser_lookahead_add(ps, parser_stack_pop(ps));

                sym = parser_fetch_symbol(ps);
                declarator = true;
        } 

        if (decltion_second[sym]) {
                parser_lookahead_set_type(ps, psym_declaration);
                status = parser_declaration(ps);
                if (!MC_SUCC(status)) {
                        status = parser_error(ps, "expected declaration");
                        goto fail;
                }

                assert(parser_stack_top(ps)->sym == psym_declaration);
                pt_node_child_add(ext_decl, parser_stack_pop(ps));

        } else if (decl_list_first[sym] && declarator) {
                parser_lookahead_set_type(ps, psym_function_definition);
                status = parser_function_definition(ps);
                if (!MC_SUCC(status)) {
                        parser_error(ps, "expected function-definition");
                        goto fail;
                }
                assert(parser_stack_top(ps)->sym == psym_function_definition);
                pt_node_child_add(ext_decl, parser_stack_pop(ps));
        } else {
                status = parser_error(ps, "unexpected ?token");
                goto fail;
        }

        /* lookahed symbol restored and consumed */
        old_lk = parser_lookahead_swap(ps, old_lk);
        assert(old_lk == NULL);

        return MC_OK;
fail:
        parser_lookahead_restore(ps, old_lk);
        parser_stack_restore(ps, ext_decl);
        return status;
}

struct pt_node *parser_translation_unit(struct parser *ps)
{
        struct pt_node *unit = pt_node_create(psym_translation_unit);
        mc_status_t status = MC_OK;
        enum parser_symbol sym;

        while (MC_SUCC(status) && (sym = parser_fetch_symbol(ps))
                != psym_invalid) {
                struct pt_node *child = NULL;
                /* single place, where there is no sence cheking 
                 * first symbol match */
                status = parser_external_declaration(ps);

                if (MC_SUCC(status)) {
                        assert(parser_stack_top(ps)->sym 
                                == psym_translation_unit);
                        child = parser_stack_pop(ps);
                        pt_node_child_add(unit, child);
                } else  {
                        status = parser_error(ps, 
                                "expected external-declaration");
                }
        }
        if (pt_node_child_empty(unit)) {
                pt_node_destroy(unit);
                unit = NULL;
        }
        
        return unit;
}

void parser_init(struct parser *ps, struct parser_ops ops, void *user_data)
{
        ps->ops = ops;
        ps->data = user_data;
        parser_stack_init(ps);
}

void parser_free(struct parser *ps)
{
        parser_stack_free(ps);
}