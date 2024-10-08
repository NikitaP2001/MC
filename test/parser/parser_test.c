#include <stdio.h>
#include <stdlib.h>
     
#include <mc.h>
#include <parser.h>
#include <test_common.h>

struct parser_context {
        struct token *curr;
        struct token *last_pull;
        struct token *last_error;
};

void parser_context_init(struct parser_context *pctx, 
                        struct convert_context *cctx)
{
        pctx->curr = convert_get_token(cctx);
        pctx->last_error = NULL;
}

void parser_context_pull(struct parser_context *pctx)
{
        struct token *curr = pctx->curr;
        if (curr != NULL)
                pctx->curr = list_next(curr);
}

struct token *pull_token(void *pp_data)
{
        struct parser_context *pctx = (struct parser_context *)pp_data;
        struct token *curr_tok = pctx->curr;
        parser_context_pull(pctx);
        return pctx->last_pull = curr_tok;
}

void put_token(void *pp_data, struct token *tok)
{
        struct parser_context *pctx = (struct parser_context *)pp_data;
        assert(list_next(tok) == pctx->curr);
        pctx->curr = tok;
}

struct token *fetch_token(void *pp_data)
{
        struct parser_context *pctx = (struct parser_context *)pp_data;
        return pctx->curr;
}

mc_status_t error_handler(void *pp_data, const char *message)
{
        UNUSED(message);
        UNUSED(pp_data);
        return MC_FAIL;
}

struct parser_test_context {
        struct filesys fs;
        struct pp_context pp;
        struct convert_context ctx;
        struct parser ps;
        struct parser_context pctx; 
};

static void parser_test_init(struct parser_test_context *t_ctx, 
                             const char *fname)
{
        fs_init(&t_ctx->fs);
        fs_add_local(&t_ctx->fs, "./");
        pp_init(&t_ctx->pp, &t_ctx->fs);

        enum mc_status status = pp_run(&t_ctx->pp, fname);
        assert(MC_SUCC(status));

        convert_init(&t_ctx->ctx, &t_ctx->pp);
        assert(MC_SUCC(convert_run(&t_ctx->ctx)));

        parser_context_init(&t_ctx->pctx, &t_ctx->ctx);
        struct parser_ops ops = {
                .pull_token     = pull_token,
                .put_token      = put_token,
                .fetch_token    = fetch_token,
                .error          = error_handler,
        };

        parser_init(&t_ctx->ps, ops, &t_ctx->pctx);
}

static void parser_test_free(struct parser_test_context *t_ctx)
{
        parser_free(&t_ctx->ps);
        convert_free(&t_ctx->ctx);
        pp_free(&t_ctx->pp);
        fs_free(&t_ctx->fs);
}

TEST_CASE(parser, test1_general_valid)
{
        struct parser_test_context t_ctx;
        parser_test_init(&t_ctx, "test1.tc");

        struct pt_node *node = parser_translation_unit(&t_ctx.ps);
        ASSERT_NE(node, NULL);

        parser_test_free(&t_ctx);
}

TEST_CASE(parser, test2_typedef_valid)
{
        struct parser_test_context t_ctx;
        parser_test_init(&t_ctx, "test2.tc");

        struct pt_node *node = parser_translation_unit(&t_ctx.ps);
        ASSERT_NE(node, NULL);

        parser_test_free(&t_ctx);
}

TEST_CASE(parser, test3_typedef_valid)
{
        struct parser_test_context t_ctx;
        parser_test_init(&t_ctx, "test3.tc");

        struct pt_node *node = parser_translation_unit(&t_ctx.ps);
        ASSERT_EQ(node, NULL);

        parser_test_free(&t_ctx);
}



int main()
{
     mc_init(0, NULL);
     TEST_RUN(parser, test1_general_valid);
     TEST_RUN(parser, test2_typedef_valid);
     TEST_RUN(parser, test3_typedef_valid);

     mc_free();
}