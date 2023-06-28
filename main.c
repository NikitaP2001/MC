#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <pp.h>


int main()
{
        char val[500];
        struct filesys fs = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./mc/pp/");

        struct fs_file *src = fs_get_local(&fs, "pp.c");
        assert(src != NULL);
        struct preproc pp = {0};
        pp_init(&pp, src);

        struct pp_token *token = NULL;
        while ((token = pp_get_token(&pp)) != NULL) {
                if (token->length < 500) {
                        pp_token_getval(token, val);
                        printf("TOKEN: %s\n", val);
                } else
                        printf("token too long: %lld\n", token->length);
                        
                pp_add_token(&pp, token);
        }
                

        pp_free(&pp);
        fs_free(&fs);
}