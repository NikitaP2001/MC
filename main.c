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
};

void parser_context_init(struct parser_context *pctx, 
                        struct convert_context *cctx)
{
        pctx->curr = convert_get_token(cctx);
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
        parser_context_pull(pctx);
        TOKEN_PRINT_CONTENT(pctx->curr);
        return pctx->curr;
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
        UNUSED(pp_data);
        printf("error: %s", message);
        return MC_FAIL;
}

enum parser_symbol get_symbol(void *pp_data, struct token *name)
{
        UNUSED(pp_data);
        UNUSED(name);
        return psym_identifier;
}

int main()
{
        mc_init();
        struct filesys fs;
        struct pp_context pp;
        struct convert_context ctx;
        struct parser ps;

        fs_init(&fs);
        fs_add_local(&fs, "./mc"); 
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
                .get_symbol = get_symbol,
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