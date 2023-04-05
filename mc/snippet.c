#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <snippet.h>
#include <tools.h>

struct snippet *snippet_create(const char *file_name)
{
        struct snippet *new_snippet = calloc(1, sizeof(struct snippet));

        int name_len = strlen(file_name);
        if (new_snippet != NULL) {
                new_snippet->file_name = (char*)malloc((name_len + 1) * sizeof(char));
                strcpy(new_snippet->file_name, file_name);
        }
        
        return new_snippet;
}

static enum mc_status read_text_file(FILE *fptr, char *buffer, int max_size)
{
        enum mc_status status = MC_FILE_OP_FAIL;
        int nread = 0;
        char cread = '\0';

        while ((cread = fgetc(fptr)) != EOF && nread < max_size)
                buffer[nread++] = cread;

        if (!ferror(fptr) && feof(fptr)) {
                buffer[nread] = '\0';
                status = MC_OK;
        }
        return status;
}


enum mc_status snippet_read_file(struct snippet *sn)
{
        enum mc_status status = MC_OPEN_FILE_FAIL;
        FILE *fptr = fopen(sn->file_name, "r");

        if (fptr != NULL) {
                int fsize = get_file_size(fptr); 
                sn->content = (char_t*)malloc(fsize + 1);
                status = MC_MEMALLOC_FAIL;
                if (sn->content != NULL)
                        status = read_text_file(fptr, sn->content, fsize);
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
