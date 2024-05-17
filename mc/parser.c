#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <parser.h>

static inline _Bool 
parser_symbol_not_terminal(enum parser_symbol sym)
{
        return (sym < psym_last_nonterm);
}

static inline _Bool
parser_symbol_is_terminal(enum parser_symbol sym)
{
        return (sym > psym_last_nonterm && sym < psym_last_term);
}

static struct parser_production parser_grammar[] = {
        { psym_postfix_expression, 4, { psym_postfix_expression,
                                        psym_left_square_bracket, 
                                        psym_expression,
                                        psym_right_square_bracket }
        },

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

static void 
parser_production_list_init(struct parser *ps, enum parser_symbol type)
{
        struct parser_production *prod = &parser_grammar[0];
        for (size_t i = 0; 
        i < sizeof(parser_grammar) / sizeof(struct parser_production); 
        i++, prod++) {
                if (prod->source != type)
                        continue;

                struct parser_production_list *p_lst 
                        = calloc(1, sizeof(struct parser_production_list *));
                p_lst->production = prod;
                if (ps->grammar[type] == NULL)
                        ps->grammar[type] = p_lst;
                else
                        list_append(ps->grammar[type], p_lst);
        }
        /* grammar may be incorrect */
        assert(ps->grammar[type] == NULL);
}

static void
parser_production_list_free(struct parser *ps)
{
        list_destroy(ps->grammar, (list_free)free);
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
        enum parser_symbol type = pr->source;
        parser_first_prod_get(ps, pr, &ps->first_set[type]);
}

static void
parser_first_init_sym(struct parser *ps, enum parser_symbol type)
{
        assert(!parser_symbol_is_terminal(type));
        struct parser_production_list *pr_list = ps->grammar[type];
        if (pr_list == NULL) {
                parser_production_list_init(ps, type);
                assert(pr_list != NULL);
        }
        LIST_FOREACH_ENTRY(pr_list) {
                parser_first_init_prod(ps, (struct parser_production*)entry);
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

        for (int i = 0; i < psym_last_nonterm; i++) {
                pr_list = ps->grammar[i];
                LIST_FOREACH_ENTRY(pr_list) {
                        if (pasrer_follow_init_prod(ps, 
                        (struct parser_production*)entry) != 0) {
                                i = 0;
                        }
                }
        } 
}

static inline
void
parser_table_row_addentry(struct parser_production *table_row, 
                         enum parser_symbol position,
                         struct parser_production entry)
{
        table_row[position - PARSER_FIRST_TERM] = entry;
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
                parser_table_row_addentry(table_row, sym, *pr);
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
                        parser_table_row_addentry(table_row, sym, pr_eps);
                }
        }
}

static void parser_table_init(struct parser *ps)
{
        struct parser_production_list *pr_list;
        struct parser_production *prod;

        for (int i = PARSER_NUM_NONTERM; i < psym_last_nonterm; i++) {
                pr_list = ps->grammar[i];

                LIST_FOREACH_ENTRY(pr_list) {
                        prod = (struct parser_production*)entry;
                        parser_table_add_production(ps, ps->table[i], prod);
                }
        } 
}

void parser_init(struct parser *ps, enum parser_symbol start_sym)
{
        memset(ps->first_set, 0, sizeof(ps->first_set));
        memset(ps->follow_set, 0, sizeof(ps->follow_set));
        memset(ps->table, 0, sizeof(ps->table));

        ps->next_sym = NULL;
        ps->get_curr_file = NULL;
        ps->get_curr_line = NULL;

        /* endmarker follows root level symbol - start_sym*/
        parser_symbol_set_insert(&ps->follow_set[start_sym], psym_endmarker);
        parser_follow_init(ps);
        parser_table_init(ps);
}

void parser_free(struct parser *ps)
{
        parser_production_list_free(ps);
}

/*
        create callbacks, call on node parse
        seperate callback for error
        that we could define some unneded callbacks to
        error handler when parsing const expr, for example
_Bool parser_build_tree(struct parser *ps, void *pp_data)
{
}
*/


