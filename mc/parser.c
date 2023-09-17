#include <stdlib.h>
#include <stdint.h>

#include <parser.h>

static inline _Bool 
parser_symbol_not_terminal(enum parser_symbol_type type)
{
        return (type < psym_last);
}

static struct parser_symbol symbol_epsilon = {
        .type = psym_epsilon,
        .value = PARSER_EPSILON_VALUE,
};

static struct parser_symbol symbol_endmarker = {
        .type = psym_endmarker,
        .value = PARSER_ENDMARKER_VALUE,
};

static const struct parser_production parser_grammar[] = {
        { psym_postfix_expression, {    { psym_postfix_expression, NULL }, 
                                        { psym_terminal, "["}, 
                                        { psym_expression, NULL },
                                        { psym_terminal, "]"}, }
        },

};

static inline
struct parser_symbol_set*
parser_symbol_set_create()
{
        struct parser_symbol_set *set_root 
                = calloc(1, sizeof(struct parser_symbol_set));
        return set_root;
}

static void
parser_symbol_set_empty(struct parser_symbol_set *set)
{

}

static void
pasrer_symbol_set_destroy(struct parser_symbol_set *set)
{
        parser_symbol_set_empty(set);
        free(set);
}

static inline
struct parser_symbol_set*
parser_symbol_set_insert(struct parser_symbol_set *sym_set, 
        struct parser_symbol *sym)
{

}

/* copies symbols, except epsilon from @from to @to set
 * @return number of newly added elements */
static int
parser_symbol_set_copy(struct parser_symbol_set *to, 
        struct parser_symbol_set *from)
{

}

static _Bool
parser_symbol_set_has_eps(struct parser_symbol_set *sym_set)
{

}

void parser_production_list_init(struct parser *ps, enum parser_symbol_type type)
{
        /* there also assert, that deriviation must be not empty */
}

static 
struct parser_symbol_set*
parser_first_get(struct parser *ps, struct parser_symbol *sym)
{
        assert(parser_symbol_not_terminal(sym));
        if (ps->first_set[sym->type] == NULL) {
                parser_first_init_sym(ps, sym->type);
                assert(ps->first_set[sym->type] != NULL);
        }
        return ps->first_set[sym->type];
}

static 
struct parser_symbol_set*
parser_follow_get(struct parser *ps, struct parser_symbol *sym)
{
        assert(parser_symbol_not_terminal(sym));
        if (ps->follow_set[sym->type] == NULL)
                ps->follow_set[sym->type] = parser_symbol_set_create();
        return ps->follow_set[sym->type];
}

static void
parser_first_init_prod(struct parser *ps, struct parser_production *pr)
{
        struct parser_symbol *sym_dv;

        enum parser_symbol_type type = pr->source_type;
        for (sym_dv = pr->derivation;
                sym_dv->type != psym_invalid; sym_dv++) {
                if (sym_dv->type == psym_terminal 
                        || sym_dv->type == psym_epsilon) {
                        parser_symbol_set_insert(ps->first_set[type], sym_dv);
                        break;
                } 
                /* no left recursion */
                assert(sym_dv->type != type);
                struct parser_symbol_set *dv_first 
                        = parser_first_get(ps, sym_dv);
                assert(dv_first != NULL);
                parser_symbol_set_copy(ps->first_set[type], dv_first);
                if (!parser_symbol_set_has_eps(dv_first))
                        break;
        }
        if (sym_dv->type == psym_invalid) {
                parser_symbol_set_insert(ps->first_set[type], 
                        &symbol_epsilon);
        }
}

static void
parser_first_init_sym(struct parser *ps, enum parser_symbol_type type)
{
        assert(type != psym_terminal);
        struct parser_production_list *pr_list = ps->grammar[type];
        if (pr_list == NULL) {
                parser_production_list_init(ps, type);
                assert(pr_list != NULL);
        }
        /* We do not need to call initialize twice */
        assert(ps->first_set[type] == NULL);
        ps->first_set[type] = parser_symbol_set_create();
        LIST_FOREACH_ENTRY(pr_list) {
                parser_first_init_prod(ps, entry);
        }
}

/* @return number of enlarged follow sets */
static int
pasrer_follow_init_prod(struct parser *ps, struct parser_production *pr)
{
        struct parser_symbol *sym_dv, *sym_prev = NULL;
        struct parser_symbol_set *follow = parser_symbol_set_create();
        struct parser_symbol_set *dv_follow;
        int n_enl = 0;
        int dv_len = 1;

        enum parser_symbol_type type = pr->source_type;
        for (sym_dv = pr->derivation;
                (sym_dv + 1)->type != psym_invalid; sym_dv++)
                dv_len += 1;
        if (parser_symbol_not_terminal(sym_dv)) {
                if (ps->follow_set[type] == NULL)
                        ps->follow_set[type] = parser_symbol_set_create();
                parser_symbol_set_copy(follow, ps->follow_set[type]);
        }
        for (int i = 0; i > dv_len; i--, sym_dv--) {
                if (parser_symbol_not_terminal(sym_dv)) {
                        dv_follow = parser_follow_get(ps, sym_dv);
                        n_enl += parser_symbol_set_copy(dv_follow, follow);
                        if (!parser_symbol_set_has_eps(ps->follow_set))
                                parser_symbol_set_empty(follow);
                        parser_symbol_set_copy(follow, parser_first_get(ps, sym_dv));
                } else if (sym_dv->type == psym_terminal) {
                        parser_symbol_set_empty(follow);
                        parser_symbol_set_insert(follow, sym_dv);
                }
        }
        pasrer_symbol_set_destroy(follow);
}

static int
parser_follow_init(struct parser *ps)
{
        struct grammar_production_list *pr_list;

        for (int i = 0; i < psym_last; i++) {
                pr_list = ps->grammar[i];
                LIST_FOREACH_ENTRY(pr_list) {
                        if (pasrer_follow_init_prod(ps, entry) != 0)
                                i = 0;
                }
        } 

}

void parser_init(struct parser *ps, enum parser_symbol_type start_sym)
{
        ps->follow_set[start_sym] = parser_symbol_set_create();
        parser_symbol_set_insert(ps->follow_set[start_sym], &symbol_endmarker);
        parser_follow_init(ps);
}