#include <assert.h>

#include <token.h>
#include <pp/node.h>
#include <pp/token.h>

struct token *token_create(struct pp_token *first, struct pp_token *last)
{
        UNUSED(first);
        UNUSED(last);
        return NULL;
}

void token_destroy(struct token *tok)
{
        UNUSED(tok);
}

struct token *token_convert(struct pp_context *pp)
{
        struct token *result = NULL;

        LIST_FOREACH_ENTRY(pp_node_leftmost_leaf(pp->root_file)) {
                struct pp_token* unit_pos = (struct pp_token*)entry;
                switch (unit_pos->type) {
                        case pp_id:
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

                                /* nothing other should be left */
                                assert(pp_token_valcmp(unit_pos, "\n") == 0);
                                break;
                        default:
                                MC_LOG(MC_CRIT, "unexpected type on this phase");
                                goto fail;
                }
        }
        return result;
fail:
        list_destroy(result, (list_free)token_destroy);
        return NULL;
}