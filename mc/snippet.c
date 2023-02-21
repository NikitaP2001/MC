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
        strcpy(new_snippet->file_name, file_name);

        return new_snippet;
}


static int get_file_size(FILE *fp)
{
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        return size;
}


_Bool snippet_read_file(struct snippet *sn)
{
        _Bool status = false;
        FILE *fptr = fopen(sn->file_name, "r");

        if (fptr != NULL) {
                int fsize = get_file_size(fptr); 

                sn->content = (char_t*)malloc(fsize + 1);
                int nread = fread(sn->content, sizeof(char), fsize, fptr);
                if (nread == fsize) {
                        sn->content[nread] = '\0';
                        status = true;
                }

                fclose(fptr);
        }
        return status;
}


void snippet_destroy(struct snippet *sn)
{
        if (sn->content != NULL)
                free(sn->content);
        free(sn->file_name);
        free(sn);
}
