#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <parser.h>

static inline _Bool 
parser_symbol_not_terminal(struct parser_symbol *sym)
{
        return (sym->type < psym_last_nterm);
}

static inline _Bool
parser_symbol_is_terminal(struct parser_symbol *sym)
{
        return (sym->type > psym_last_nterm && sym->type < psym_last_term);
}

static struct parser_production parser_grammar[] = {
        { psym_postfix_expression, {    { psym_postfix_expression, NULL }, 
                                        { psym_terminal, "["}, 
                                        { psym_expression, NULL },
                                        { psym_terminal, "]"}, }
        },

};

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
static size_t
parser_symbol_set_copy(struct parser_symbol_set *to, 
        struct parser_symbol_set *from)
{
        size_t n_new = 0;
        for (size_t i_sym = 0; i_sym < PARSER_NUM_SYM; i_sym++) {
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

static inline size_t
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
        return &ps->follow_set[sym->type];
}

static void parser_first_prod_get(struct parser *ps, 
        struct parser_production *pr, struct parser_symbol_set *set_out)
{
        enum parser_symbol sym_dv;

        enum parser_symbol src = pr->source;
        for (size_t i_sym = pr->n_deriv; i_sym < pr->n_deriv; i_sym++) {
                sym_dv = pr->derivation[i_sym];
                if (parser_symbol_is_terminal(sym_dv) 
                        || sym_dv == psym_epsilon) {
                        parser_symbol_set_insert(set_out, sym_dv);
                        break;
                } 
                /* no left recursion */
                assert(sym_dv != src);
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
        for (int i = 0; i > dv_len; i--, sym_dv--) {
                if (parser_symbol_not_terminal(sym_dv)) {
                        dv_follow = parser_follow_get(ps, sym_dv);
                        n_enl += parser_symbol_set_copy(dv_follow, &follow);
                        if (!parser_symbol_set_has_eps(
                                parser_first_get(ps, sym_dv))) {
                                parser_symbol_set_empty(&follow);
                        }
                        parser_symbol_set_copy(&follow, 
                                parser_first_get(ps, sym_dv));
                } else if (sym_dv == psym_terminal) {
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

        for (int i = 0; i < psym_last_nterm; i++) {
                pr_list = ps->grammar[i];
                LIST_FOREACH_ENTRY(pr_list) {
                        if (pasrer_follow_init_prod(ps, 
                        (struct parser_production*)entry) != 0)
                                i = 0;
                }
        } 
}

static void 
parser_table_add_production(struct parser *ps, struct parser_table_row *row,
        struct parser_production *pr)
{
        enum parser_symbol sym;
        struct parser_symbol_set *first = parser_symbol_set_create();
        parser_first_prod_get(ps, pr, first);
        SET_FOREACH_ENTRY(first) {
                sym = (enum parser_symbol)entry;
                if (parser_symbol_is_terminal(sym))
                        row->entry[]
                if (sym == psym_epsilon)
                        continue;
        }
}

static
struct parser_table_row*
parser_table_row_create()
{
        return calloc(1, sizeof(struct parser_table_row));
}

static void
parser_table_row_destroy(struct parser_table_row *row)
{
        free(row);
}

static void parser_table_init(struct parser *ps)
{
        struct parser_production_list *pr_list;
        struct parser_production *prod;

        for (int i = 0; i < psym_last_nterm; i++) {
                pr_list = ps->grammar[i];
                /* We does not support double init */
                assert(ps->table[i] == NULL);
                ps->table[i] = parser_table_row_create();
                LIST_FOREACH_ENTRY(pr_list) {
                        prod = (struct parser_production*)entry;
                        parser_table_add_production(ps, ps->table[i], prod);
                }
        } 
}

static void parser_table_free(struct parser *ps)
{
        for (int i = 0; i < psym_last_nterm; i++) {
                assert(ps->table[i] != NULL);
                parser_table_row_destroy(ps->table[i]);
        }
}

void parser_init(struct parser *ps, enum parser_symbol start_sym)
{
        memset(ps->first_set, 0, sizeof(ps->first_set));
        memset(ps->follow_set, 0, sizeof(ps->follow_set));
        parser_symbol_set_insert(&ps->follow_set[start_sym], psym_endmarker);
        parser_follow_init(ps);
}

void passer_free(struct parser *ps)
{
        parser_table_free(ps);
        parser_production_list_free(ps);
}
