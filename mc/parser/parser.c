#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <parser.h>
#include "grammar.h"

const char *parser_symbol_nonterms[] = {
        "psym_expression",
        "psym_expression2",
        "psym_conditional_expression",
        "psym_logical_or_expression",
        "psym_logical_or_expression2",
        "psym_logical_and_expression",
        "psym_logical_and_expression2",
        "psym_inclusive_or_expression",
        "psym_inclusive_or_expression2",
        "psym_exclusive_or_expression",
        "psym_exclusive_or_expression2",
        "psym_and_expression",
        "psym_and_expression2",
        "psym_equality_expression",
        "psym_equality_expression2",
        "psym_relational_expression",
        "psym_relational_expression2",
        "psym_shift_expression",
        "psym_shift_expression2",
        "psym_additive_expression",
        "psym_additive_expression2",
        "psym_multiplicative_expression",
        "psym_multiplicative_expression2",
        "psym_cast_expression",
        "psym_unary_expression",
        "psym_type_name",
        "psym_postfix_expression",
        "psym_postfix_expression2",
        "psym_unary_operator",
        "psym_primary_expression",
        "psym_initializer_list",
        "psym_initializer_list2",
        "psym_argument_expression_list",
        "psym_argument_expression_list2",
        "psym_assignment_expression",
        "psym_assignment_operator",
        "psym_constant_expression",
        "psym_specifier_qualifier_list",
        "psym_abstract_declarator",
        "psym_type_specifier",
        "psym_type_qualifier",
        "psym_struct_or_union_specifier",
        "psym_enum_specifier",
        "psym_typedef_name",
        "psym_enumerator_list",
        "psym_enumerator_list2",
        "psym_enumerator",
        "psym_enumeration_constant",
        "psym_designation",
        "psym_initializer",
        "psym_designator_list",
        "psym_designator_list2",
        "psym_designator",
        "psym_pointer",
        "psym_direct_abstract_declarator",
        "psym_direct_abstract_declarator2",
        "psym_parameter_type_list",
        "psym_struct_or_union",
        "psym_struct_declaration_list",
        "psym_struct_declaration_list2",
        "psym_type_qualifier_list",
        "psym_type_qualifier_list2",
        "psym_struct_declaration",
        "psym_struct_declarator_list",
        "psym_struct_declarator_list2",
        "psym_struct_declarator",
        "psym_translation_unit",
        "psym_translation_unit2",
        "psym_external_declaration",
        "psym_parameter_list",
        "psym_parameter_list2",
        "psym_parameter_declaration",
        "psym_declaration_specifiers",
        "psym_declaration_specifiers_opt",
        "psym_declarator",
        "psym_storage_class_specifier",
        "psym_function_specifier",
        "psym_function_definition",
        "psym_declaration",
        "psym_direct_declarator",
        "psym_direct_declarator2",
        "psym_declaration_list",
        "psym_declaration_list2",
        "psym_compound_statement",
        "psym_init_declarator_list",
        "psym_init_declarator_list2",
        "psym_block_item_list",
        "psym_block_item_list2",
        "psym_init_declarator",
        "psym_block_item",
        "psym_statement",
        "psym_labeled_statement",
        "psym_expression_statement",
        "psym_selection_statement",
        "psym_iteration_statement",
        "psym_jump_statement",
        "psym_identifier_list",
        "psym_identifier_list2",
};

/* @return next symbol present in set, ordered after @sym_num, 
 * psym_invalid in case no more symbols present */
static inline 
int 
parser_symbol_next_sym(const struct parser_symbol_set *sym_set, int sym_num)
{
        sym_num += 1;
        for ( ; sym_num < PARSER_NUM_SYM; sym_num++) {
                if (sym_set->syms[sym_num])
                        return sym_num;
        }
        return psym_invalid;
}

#define PARSER_SYM_SET_FOREACH(set)                                         \
        for (int entry = parser_symbol_next_sym(set, psym_last_nonterm);    \
        entry != psym_invalid;                                              \
        entry = parser_symbol_next_sym(set, entry))

static void
parser_symbol_set_empty(struct parser_symbol_set *set)
{
        memset(set->syms, 0, sizeof(set->syms));
        set->n_elem = 0;
}

static inline void
parser_symbol_set_insert(struct parser_symbol_set *sym_set, 
                         enum parser_symbol sym)
{
        if (sym_set->syms[sym] == false)
                sym_set->n_elem += 1;
        sym_set->syms[sym] = true;
}

/* copies symbols, except epsilon from @from to @to set
 * @return number of newly added elements */
static int
parser_symbol_set_copy(struct parser_symbol_set *to, 
        struct parser_symbol_set *from)
{
        int n_new = 0;
        for (int i_sym = 0; i_sym < PARSER_NUM_SYM; i_sym++) {
                if (from->syms[i_sym]) {
                        if (to->syms[i_sym] == false) {
                                to->n_elem += 1;
                                n_new += 1;
                        }
                        to->syms[i_sym] = true;
                }
        }
        return n_new;
}

static inline _Bool
parser_symbol_set_has_eps(struct parser_symbol_set *sym_set)
{
        return sym_set->syms[psym_epsilon];
}

static inline int
parser_symbol_set_size(struct parser_symbol_set *sym_set)
{
        return sym_set->n_elem;
}

static _Bool parser_symbol_set_is_joint(struct parser_symbol_set *set1, 
                                        struct parser_symbol_set *set2)
{
        if (set1->n_elem == 0 || set2->n_elem == 0)
                return false;
        for (uint32_t i = 0; i < PARSER_NUM_SYM; i++) {
                if (set1->syms[i] == set2->syms[i])
                        return true;
        }
        return false;
}

static void
parser_grammar_add(struct parser *ps, struct parser_production_list *p_lst)
{
        enum parser_symbol type = p_lst->production->source;
        int gram_pos = type - psym_first_nonterm;
        if (ps->grammar[gram_pos] == NULL)
                ps->grammar[gram_pos] = p_lst;
        else
                list_append(ps->grammar[gram_pos], p_lst);
}

static inline 
struct parser_production_list* 
parser_grammar_get(struct parser *ps, enum parser_symbol type)
{
        int gram_pos = type - psym_first_nonterm;
        return ps->grammar[gram_pos];
}

static void 
parser_production_list_init(struct parser *ps, enum parser_symbol type)
{
        struct parser_production *prod = &parser_grammar[0];
        size_t grammar_size = parser_grammar_size();
        for (size_t i = 0; i < grammar_size; i++, prod++) {
                if (prod->source != type)
                        continue;

                struct parser_production_list *p_lst 
                        = calloc(1, sizeof(struct parser_production_list));
                p_lst->production = prod;
                parser_grammar_add(ps, p_lst);
        }
        /* grammar may be incorrect */
        assert(parser_grammar_get(ps, type) != NULL);
}

static inline _Bool
parser_production_is_invalid(struct parser_production prod)
{
        return prod.source == psym_invalid;
}

static void
parser_production_list_free(struct parser *ps)
{
        for (int i = 0; i < PARSER_NUM_NONTERM; i++) {
                if (ps->grammar[i] != NULL)
                        list_destroy(ps->grammar[i], (list_free)free);
        }
}

static void
parser_first_init_sym(struct parser *ps, enum parser_symbol type);

static 
struct parser_symbol_set*
parser_first_get(struct parser *ps, enum parser_symbol sym)
{
        struct parser_symbol_set *sym_set = &ps->first_set[sym];
        assert(parser_symbol_not_terminal(sym));
        if (parser_symbol_set_size(sym_set) == 0) {
                parser_first_init_sym(ps, sym);
                assert(parser_symbol_set_size(sym_set) != 0);
        }
        return &ps->first_set[sym];
}

static inline
struct parser_symbol_set*
parser_follow_get(struct parser *ps, enum parser_symbol sym)
{
        assert(parser_symbol_not_terminal(sym));
        return &ps->follow_set[sym];
}

static void parser_first_prod_get(struct parser *ps, 
                                  struct parser_production *pr, 
                                  struct parser_symbol_set *set_out)
{
        enum parser_symbol sym_dv;

        /* no left recursion */
        assert(pr->derivation[0] != pr->source);
        for (size_t i_sym = 0; i_sym < pr->n_deriv; i_sym++) {
                sym_dv = pr->derivation[i_sym];
                if (parser_symbol_is_terminal(sym_dv) 
                        || sym_dv == psym_epsilon) {
                        parser_symbol_set_insert(set_out, sym_dv);
                        break;
                } 
                /* no left recursion */
                assert(sym_dv != pr->source);
                struct parser_symbol_set *dv_first 
                        = parser_first_get(ps, sym_dv);
                assert(dv_first != NULL);
                parser_symbol_set_copy(set_out, dv_first);
                if (!parser_symbol_set_has_eps(dv_first))
                        break;
        }
        if (parser_symbol_set_size(set_out) == 0)
                parser_symbol_set_insert(set_out, psym_epsilon);
}

static void
parser_first_init_prod(struct parser *ps, struct parser_production *pr)
{
        struct parser_symbol_set pr_set = {0};
        enum parser_symbol type = pr->source;
        parser_first_prod_get(ps, pr, &pr_set);
        if (parser_symbol_set_is_joint(&ps->first_set[type], &pr_set))
                MC_LOG(MC_ERR, "LL1 rule does not hold for %s", 
                        parser_symbol_to_str(type));
        parser_symbol_set_copy(&ps->first_set[type], &pr_set);
}

static void
parser_first_init_sym(struct parser *ps, enum parser_symbol type)
{
        struct parser_production_list *pr_entry;
        assert(!parser_symbol_is_terminal(type));
        struct parser_production_list *pr_list = parser_grammar_get(ps, type);
        if (pr_list == NULL) {
                parser_production_list_init(ps, type);
                pr_list = parser_grammar_get(ps, type);
                assert(pr_list != NULL);
        }
        LIST_FOREACH_ENTRY(pr_list) {
                pr_entry = (struct parser_production_list*)entry;
                parser_first_init_prod(ps, pr_entry->production);
        }
}

/* @return number of enlarged follow sets */
static int
pasrer_follow_init_prod(struct parser *ps, struct parser_production *pr)
{
        struct parser_symbol_set follow = {0};
        struct parser_symbol_set *dv_follow;
        int n_enl = 0;
        size_t dv_len = pr->n_deriv;
        enum parser_symbol sym_dv = pr->derivation[dv_len - 1];
        enum parser_symbol src = pr->source;

        if (parser_symbol_not_terminal(sym_dv))
                parser_symbol_set_copy(&follow, &ps->follow_set[src]);
        for (size_t i = 0; i > dv_len; i--, sym_dv--) {
                if (parser_symbol_not_terminal(sym_dv)) {
                        dv_follow = parser_follow_get(ps, sym_dv);
                        n_enl += parser_symbol_set_copy(dv_follow, &follow);
                        if (!parser_symbol_set_has_eps(
                                parser_first_get(ps, sym_dv))) {
                                parser_symbol_set_empty(&follow);
                        }
                        parser_symbol_set_copy(&follow, 
                                parser_first_get(ps, sym_dv));
                } else if (parser_symbol_is_terminal(sym_dv)) {
                        parser_symbol_set_empty(&follow);
                        parser_symbol_set_insert(&follow, sym_dv);
                }
        }
        return n_enl;
}

static void
parser_follow_init(struct parser *ps)
{
        struct parser_production_list *pr_list;
        struct parser_production_list *pr_entry;

        for (int i = psym_first_nonterm; i < psym_last_nonterm; i++) {
                parser_first_init_sym(ps, i);
                pr_list = parser_grammar_get(ps, i);
                LIST_FOREACH_ENTRY(pr_list) {
                        pr_entry = (struct parser_production_list*)entry;
                        if (pasrer_follow_init_prod(ps, 
                                pr_entry->production) != 0) {
                                i = 0;
                        }
                }
        } 
}

static inline
struct parser_production
parser_table_row_get_entry(struct parser_production *table_row, 
                         enum parser_symbol position)
{
        static struct parser_production invalid = {
                .source = psym_invalid
        };
        if (parser_symbol_is_terminal(position))
                return table_row[position - psym_first_term];
        else
                return invalid;
}

static inline void
parser_table_row_add_entry(struct parser_production *table_row, 
                         enum parser_symbol position,
                         struct parser_production entry)
{
        table_row[position - psym_first_term] = entry;
}

static void 
parser_table_add_production(struct parser *ps, 
                            struct parser_production *table_row,
                            struct parser_production *pr)
{
        enum parser_symbol sym;
        struct parser_symbol_set first = {0};
        struct parser_symbol_set *follow;
        _Bool has_eps = false;

        parser_first_prod_get(ps, pr, &first);
        PARSER_SYM_SET_FOREACH(&first) {
                sym = (enum parser_symbol)entry;
                if (sym == psym_epsilon) {
                        has_eps = true;
                        continue;
                }
                parser_table_row_add_entry(table_row, sym, *pr);
        }
        if (has_eps) {
                struct parser_production pr_eps = {
                        .source = pr->source,
                        .n_deriv = 1,
                };
                pr_eps.derivation[0] = psym_epsilon;
                follow = parser_follow_get(ps, pr->source);
                PARSER_SYM_SET_FOREACH(follow) {
                        sym = (enum parser_symbol)entry;
                        parser_table_row_add_entry(table_row, sym, pr_eps);
                }
        }
}

struct parser_production*
parser_table_get(struct parser *ps, enum parser_symbol type)
{
        int row_pos = type - psym_first_nonterm;
        return ps->table[row_pos];
}

static void parser_table_init(struct parser *ps)
{
        struct parser_production_list *pr_list;
        struct parser_production_list *pr_entry;

        for (int i = psym_first_nonterm; i < psym_last_nonterm; i++) {
                pr_list = parser_grammar_get(ps, i);

                LIST_FOREACH_ENTRY(pr_list) {
                        pr_entry = (struct parser_production_list*)entry;
                        parser_table_add_production(ps, 
                                parser_table_get(ps, i), pr_entry->production);
                }
        } 
}

void parser_init(struct parser *ps, 
                struct parser_ops ops,
                enum parser_symbol start_sym)
{
        memset(ps->first_set, 0, sizeof(ps->first_set));
        memset(ps->follow_set, 0, sizeof(ps->follow_set));
        memset(ps->table, 0, sizeof(ps->table));
        memset(ps->grammar, 0, sizeof(ps->grammar));

        ps->ops = ops;
        ps->start_sym = start_sym;

        /* endmarker follows root level symbol - start_sym*/
        parser_symbol_set_insert(&ps->follow_set[start_sym], psym_endmarker);
        parser_follow_init(ps);
        parser_table_init(ps);
}

void parser_free(struct parser *ps)
{
        parser_production_list_free(ps);
}

#define PARSER_STACK_SIZE_INIT 1000
#define PARSER_STACK_GR_FACTOR 2

struct parser_stack {
        enum parser_symbol *data;
        size_t capacity;
        size_t size;
};

static void 
parser_stack_init(struct parser *ps, struct parser_stack *stack)
{
        stack->data = calloc(PARSER_STACK_SIZE_INIT, 
                sizeof(enum parser_symbol));
        stack->capacity = PARSER_STACK_SIZE_INIT;
        stack->size = 1;
        stack->data[0] = ps->start_sym;
}

static void parser_stack_free(struct parser_stack *stack)
{
        free(stack->data);
}

static enum parser_symbol parser_stack_top(struct parser_stack *stack)
{
        if (stack->size > 0)
                return stack->data[stack->size - 1];
        else
                return psym_endmarker;
}

static void 
parser_stack_push(struct parser_stack *stack, enum parser_symbol sym)
{
        assert(stack->size <= stack->capacity);
        if (stack->size == stack->capacity) {
                stack->capacity *= PARSER_STACK_GR_FACTOR;
                stack->data = realloc(stack->data, stack->capacity
                        * sizeof(enum parser_symbol));
        }
        stack->data[stack->size++] = sym;
}

static void parser_stack_pop(struct parser_stack *stack)
{
        assert(stack->size > 0);
        stack->size -= 1;
}

static enum parser_symbol parser_token_tosymbol(struct token *tok)
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

enum mc_status parser_process(struct parser *ps, void *pp_data)
{
        enum parser_symbol top_sym;
        struct parser_ops *pops = &ps->ops;
        struct token *curr_tok = pops->next_sym(pp_data);
        struct parser_stack ps_stack;
        enum mc_status status = MC_OK;

        parser_stack_init(ps, &ps_stack);

        while ((top_sym = parser_stack_top(&ps_stack)) 
                && top_sym != psym_endmarker
                && curr_tok != NULL && MC_SUCC(status)) {
                enum parser_symbol tok_sym = parser_token_tosymbol(curr_tok);
                if (tok_sym == top_sym) {
                        parser_stack_pop(&ps_stack);
                        curr_tok = pops->next_sym(pp_data);
                } else if (parser_symbol_is_terminal(top_sym)
                        || parser_production_is_invalid(
                        parser_table_row_get_entry(parser_table_get(ps, 
                                top_sym), tok_sym))) {
                        status = pops->error(pp_data, top_sym);
                } else {
                        struct parser_production prod = 
                                parser_table_row_get_entry(parser_table_get(ps, 
                                        top_sym), tok_sym);
                        pops->output(pp_data, prod);
                        parser_stack_pop(&ps_stack);
                        assert(prod.n_deriv > 0);
                        for (size_t i_sym = prod.n_deriv; 
                                i_sym > 0; i_sym--) {
                                parser_stack_push(&ps_stack, 
                                        prod.derivation[i_sym - 1]);
                        }
                }
        }

        parser_stack_free(&ps_stack);
        return status;
}