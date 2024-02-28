#include <assert.h>
#include <string.h>

#include <tools.h>
#include <token.h>
#include <pp/node.h>
#include <pp/token.h>

static struct token *token_create(struct pp_token *first)
{
        struct token *tok = malloc(sizeof(struct token));
        memset(tok, sizeof(struct token), 0);
        tok->first = first;
}

void token_destroy(struct token *tok)
{
        UNUSED(tok);
}

/* order should match enum token_type */
static const char* keywords[] = {
        "auto",
        "enum",
        "restrict",
        "break",
        "extern",
        "return",
        "unsigned",
        "void",
        "case",
        "float",
        "short",
        "char",
        "for",
        "signed",
        "volatile",
        "while",
        "const",
        "goto",
        "sizeof",
        "_Bool",
        "continue",
        "if",
        "static",
        "_Complex",
        "default",
        "inline",
        "struct",
        "_Imaginary",
        "do",
        "int",
        "switch",
        "double",
        "long",
        "typedef",
        "else",
        "register",
        "union"
};

static uint8_t keyw_table[PEARSON_TABLE_SIZE];

static struct token*
token_identifier(struct pp_token *unit_pos)
{
        struct token *tok = token_create(unit_pos);
        char *id_val = unit_pos->value;
        if (tok == NULL)
                return NULL;

        uint8_t hash = pearson_hash(id_val, unit_pos->length);
        int keyw_num = keyw_table[hash];

        if (keyw_num != keyw_invalid && pp_token_valcmp(unit_pos, 
                keywords[keyw_num]) == 0) {
                tok->type = tok_keyword;
                tok->value.var_keyw = keywords[keyw_num];
        } else {
                tok->type = tok_identifier;
                tok->value.var_raw.value = unit_pos->value;
        }
        return tok;
}

static struct token*
token_str_literal(struct pp_token *unit_pos)
{
        struct token *tok = token_create(unit_pos);

}

struct token *token_convert(struct pp_context *pp)
{
        struct token *result = NULL, *head = NULL;
        assert(mc_isinit());
        LIST_FOREACH_ENTRY(pp_node_leftmost_leaf(pp->root_file)) {
                struct pp_token* unit_pos = (struct pp_token*)entry;
                struct token* new_tok;
                switch (unit_pos->type) {
                        case pp_id:
                                new_tok = token_identifier(unit_pos);
                                break;
                        case pp_number:
                                break;
                        case pp_chr_const:
                                break;
                        case pp_str_lit:
                                break;
                        case pp_punct:
                                break;
                        case pp_other:
                        default:
                                MC_LOG(MC_CRIT, "unexpected token");
                                goto fail;
                }
                if (new_tok != NULL) {
                        if (result == NULL)
                                head = result = new_tok;
                        else {
                                list_append(head, new_tok);
                                head = new_tok;
                        }
                }
        }
        return result;
fail:
        list_destroy(result, (list_free)token_destroy);
        return NULL;
}

void token_init()
{
        assert(keyw_invalid < UINT8_MAX);
        memset(keyw_table, keyw_invalid, sizeof(keyw_table));
        for (size_t n_kw = 0; n_kw < ARRAY_SIZE(keywords); n_kw++) {
                const char *keyw = keywords[n_kw];
                uint8_t hash = pearson_hash(keyw, strlen(keyw));
                keyw_table[hash] = n_kw;
        }
}