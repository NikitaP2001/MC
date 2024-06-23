#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <mc.h>
#include <pp.h>
#include <parser.h>
#include <token.h>

int main()
{
        mc_init();
        struct filesys fs;
        struct pp_context pp;
        struct convert_context ctx;

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
        struct token *tok;
        tok = convert_get_token(&ctx);
        while (tok) {
                for (size_t i = 0; i < tok->first->length; i++)
                        putchar(tok->first->value[i]);
                putchar('\n');
                tok = list_next(tok);
        }

        convert_free(&ctx);
        pp_free(&pp);
        fs_free(&fs);
        puts("It`s not time yet");
}