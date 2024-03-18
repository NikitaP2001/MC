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

        fs_init(&fs);
        fs_add_local(&fs, "./mc"); 
        fs_add_global(&fs, "/usr/include"); 
        fs_add_global(&fs, "./include"); 
        fs_add_global(&fs, "/usr/lib/gcc/x86_64-linux-gnu/9/include/"); 
        fs_add_global(&fs, "B:/Install/MinGW/mingw64/x86_64-w64-mingw32/include"); 
        fs_add_global(&fs, "B:/Install/MinGW/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include"); 

        pp_init(&pp, &fs);

        enum mc_status status = pp_run(&pp, "parser.c");
        assert(MC_SUCC(status));

        pp_free(&pp);
        fs_free(&fs);
        puts("It`s not time yet");
}