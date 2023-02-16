#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <snippet.h>

struct snippet *snippet_create(const char *file_name)
{
        struct snippet *new_snippet = calloc(1, sizeof(struct snippet));

        int name_len = strlen(file_name);
        new_snippet->file_name = (char*)malloc((name_len + 1) * sizeof(char));
        strcpy((char*)new_snippet->file_name, file_name);

        return new_snippet;
}


_Bool snippet_read_file(struct snippet *sn)
{
        _Bool status = false;
        FILE *fptr = fopen(sn->file_name, "r");

        if (fptr != NULL) {
                fseek(fptr, 0, SEEK_END);
                int fsize = ftell(fptr);
                fseek(fptr, 0, SEEK_SET);

                sn->content = (char_t*)malloc(fsize + 1);
                int nread = fread(sn->content, sizeof(char_t), fsize, fptr);
                if (nread == fsize)
                        status = true;

                fclose(fptr);
        }
        return status;
}


void snippet_destroy(struct snippet *sn)
{
        if (sn->content != NULL)
                free(sn->content);

        free(sn);
}
