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

struct token *parser_context_next(struct parser_context *pctx)
{
        struct token *curr = pctx->curr;
        if (curr != NULL)
                pctx->curr = list_next(curr);
        return curr;
}

struct token *next_sym(void *pp_data)
{
        struct parser_context *pctx = (struct parser_context *)pp_data;
        struct token *tok = parser_context_next(pctx);
        TOKEN_PRINT_CONTENT(tok);
        return tok;
}

enum mc_status error(void *pp_data, enum parser_symbol top)
{
        UNUSED(pp_data);
        UNUSED(top);
        MC_LOG(MC_DEBUG, "error");

        return MC_FAIL;
}


void output(void *pp_data, struct parser_production prod)
{
        UNUSED(pp_data);
        UNUSED(prod);
        MC_LOG(MC_DEBUG, "Output production: %s", 
                parser_symbol_to_str(prod.source));
        for (size_t i = 0; i < prod.n_deriv; i++)
                printf("%s ", parser_symbol_to_str(prod.derivation[i]));
        putchar('\n');
}


int main()
{
        mc_init();
        struct filesys fs;
        struct pp_context pp;
        struct convert_context ctx;
        struct parser_context pctx;
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

        struct parser_ops ops = {
                .next_sym = next_sym,
                .error = error,
                .output = output,
        };

        parser_context_init(&pctx, &ctx);

        parser_init(&ps, ops, psym_translation_unit);
        parser_process(&ps, &pctx);
        parser_free(&ps);

        convert_free(&ctx);
        pp_free(&pp);
        fs_free(&fs);
        mc_free();
        puts("It`s not time yet");
}