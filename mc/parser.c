#include <stdlib.h>
#include <stdint.h>

#include <list.h>

enum parser_symbol_type {
        psym_invalid = 0,

        psym_constant,
        psym_integer_constant,
        psym_floating_constant,
        psym_enumeration_constant,
        psym_character_constant,

        psym_expression,
        psym_postfix_expression,

        psym_endmarker,
        psym_epsilon,
        psym_terminal,

        psym_last,
};

#define PARSER_EPSILON_VALUE "e"
#define PARSER_ENDMARKER_VALUE "$"

/* is production or terminal */
struct parser_symbol {
        enum parser_symbol_type type;
        char *value;
};

struct parser_symbol symbol_epsilon = {
        .type = psym_terminal,
        .value = PARSER_EPSILON_VALUE,
};

#define PARSER_PROD_LEN_LIMIT 4

struct parser_production {
        enum parser_symbol_type source_type;
        struct parser_symbol derivation[PARSER_PROD_LEN_LIMIT];
};

struct pasrser_production_list {
        struct list_head link;
        struct parser_production *production;
};

static const struct parser_production parser_grammar[] = {
        { psym_postfix_expression, {    { psym_postfix_expression, NULL }, 
                                        { psym_terminal, "["}, 
                                        { psym_expression, NULL },
                                        { psym_terminal, "]"}, }
        },

};

/* this should be real set, but for now it`s list */
struct parser_symbol_set {
        struct list_head *link;
        struct parser_symbol *sym;
};

static inline
struct parser_symbol_set*
parser_symbol_set_create()
{
        struct parser_symbol_set *set_root 
                = calloc(1, sizeof(struct parser_symbol_set));
        return set_root;
}

static inline
struct parser_symbol_set*
parser_symbol_set_insert(struct parser_symbol_set *sym_set, 
        struct parser_symbol *sym)
{

}

/* copies symbols, except epsilon from @from to @to set*/
static void
parser_symbol_set_copy(struct parser_symbol_set *to, 
        struct parser_symbol_set *from)
{

}

static _Bool
parser_symbol_set_has_eps(struct parser_symbol_set *sym_set)
{

}

struct parser {

        struct parser_production_list *grammar[psym_last];
        
        struct parser_symbol_set *first_set[psym_last];

        struct parser_symbol_set *follow_set[psym_last];

};

void parser_init(struct parser *ps)
{
        ps->follow_set[psym_endmarker] = parser_symbol_set_create();
        parser_symbol_set_insert()
}

void parser_production_list_init(struct parser *ps, enum parser_symbol_type type)
{
        /* there also assert, that deriviation must be not empty */
}

static 
struct parser_symbol_set*
parser_first_get(struct parser *ps, struct parser_symbol *sym)
{
        assert(sym->type != psym_terminal);
        if (ps->first_set[sym->type] == NULL) {
                parser_first_init_sym(ps, sym->type);
                assert(ps->first_set[sym->type] != NULL);
        }
        return ps->first_set[sym->type];
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

static void
parser_follow_init_sym(struct parser *ps, enum parser_symbol_type type)
{

}