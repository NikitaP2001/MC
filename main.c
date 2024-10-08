#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <mc.h>
#include <pp.h>
#include <parser.h>
#include <token.h>

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
        TOKEN_PRINT_CONTENT(curr_tok);
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
        struct parser_context *pctx = (struct parser_context *)pp_data;
        if (pctx->last_error != pctx->curr) {
                printf("error: %s", message);
                if (pctx->last_pull != NULL)
                        token_print(pctx->last_pull);
                else
                        token_print(pctx->curr);
                pctx->last_error = pctx->curr;
        }
        return MC_FAIL;
}

int main(int argc, char *argv[])
{
        mc_init(argc, argv);
        struct filesys fs;
        struct pp_context pp;
        struct convert_context ctx;
        struct parser ps;

        fs_init(&fs);
        fs_add_local(&fs, "./compiler"); 
        fs_add_global(&fs, "/usr/include"); 
        fs_add_global(&fs, "./include"); 
        fs_add_global(&fs, "/usr/lib/gcc/x86_64-linux-gnu/9/include/"); 
        fs_add_global(&fs, "B:/Install/MinGW/mingw64/x86_64-w64-mingw32/include"); 
        fs_add_global(&fs, "B:/Install/MinGW/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include"); 

        pp_init(&pp, &fs);
        enum mc_status status = pp_run(&pp, "test.c");
        if (!MC_SUCC(status)) {
                puts("Bad preprocessing");
                exit(1);
        }

        convert_init(&ctx, &pp);
        status = convert_run(&ctx);
        if (!MC_SUCC(status)) {
                puts("Bad token convertation");
                exit(1);
        }
        struct parser_context pctx; 
        parser_context_init(&pctx, &ctx);
        struct parser_ops ops = {
                .pull_token = pull_token,
                .put_token = put_token,
                .fetch_token = fetch_token,
                .error = error_handler,
        };
        parser_init(&ps, ops, &pctx);
        parser_translation_unit(&ps);

        parser_free(&ps);

        convert_free(&ctx);
        pp_free(&pp);
        fs_free(&fs);
        mc_free();
        puts("It`s not time yet");
}