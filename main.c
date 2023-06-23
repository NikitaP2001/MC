#include <stdlib.h>

#include <fs.h>
#include <pp.h>


int main()
{
        struct filesys fs = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./mc/tools.c");
        struct fs_file *src = fs_get_local(&fs, "mc.c");
        struct preproc pp = {0};
        pp_init(&pp, src);

        struct pp_token *token = NULL;
        while ((token = pp_get_token(&pp)) != NULL)
                // add tokens to pp
                ;

        pp_free(&pp);
        fs_free(&fs);
}