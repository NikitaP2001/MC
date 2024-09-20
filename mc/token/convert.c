#include <assert.h>

#include <token.h>

void convert_setpos(struct convert_context *ctx, struct pp_token *tok)
{
        ctx->pos = tok;
}

_Bool convert_advance(struct convert_context *ctx)
{
        if (!list_has_next(ctx->pos))
                return false;
        ctx->pos = (struct pp_token*)list_next(ctx->pos);
        return true;
}

void convert_retreat(struct convert_context *ctx)
{
        assert(list_has_prev(ctx->pos));
        ctx->pos = (struct pp_token*)list_prev(ctx->pos);
}

void convert_init(struct convert_context *ctx, struct pp_context *pp)
{
        assert(mc_isinit());
        ctx->pos = pp_node_leftmost_leaf(pp->root_file);
        ctx->status = MC_OK;
        ctx->first = ctx->last = NULL;
}

static void convert_append(struct convert_context *ctx, struct token *new)
{
        if (ctx->first == NULL)
                ctx->first = ctx->last = new;
        else {
                list_append(ctx->last, new);
                ctx->last = new;
        }
}

void convert_error(struct convert_context *ctx, const char *format, ...)
{
        va_list args;
        struct pp_token *tok_beg = ctx->pos;
        ctx->status = MC_SYNTAX_ERR;

        printf("%s:%lu: ", fs_file_path(tok_beg->src_file), 
                pp_token_line(tok_beg));

        va_start(args, format);
        vprintf(format, args);
        putchar('\n');
        pp_print_token_line(tok_beg);

        va_end(args);
}

enum mc_status convert_run(struct convert_context *ctx)
{
        do {
                struct token* new_tok = token_convert_next(ctx);
                if (new_tok != NULL)
                        convert_append(ctx, new_tok);
        } while (convert_advance(ctx) && convert_continue(ctx));
                
        return ctx->status;
}

struct token *convert_get_token(struct convert_context *ctx)
{
        assert(MC_SUCC(ctx->status));
        return ctx->first;
}

void convert_free(struct convert_context *ctx)
{
        if (ctx->first != NULL)
                list_destroy(ctx->first, (list_free)token_destroy);
}