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


/* @offset - number of elements, which will be left in source.
 * the rest are moved to @result */
static struct snippet *snippet_cut(struct snippet *source, size_t offset)
{
        size_t cont_len = strlen(source->content);
        if (offset == 0)
                return source;
        if (offset >= cont_len)
                return NULL;

        struct snippet *new_sn = snippet_create(source->file_name);
        new_sn->line_number = source->line_number;
        new_sn->content = malloc(cont_len - offset + 1);
        strcpy(new_sn->content, source->content + offset);

        char_t *move_pos = source->content;
        for (size_t i = 0; i < offset && *move_pos != '\0'; i++) {
                if (*move_pos++ == '\n')
                        new_sn->line_number += 1;
        }
        char_t *src_cont = malloc(offset + 1);
        strncpy(src_cont, source->content, offset);
        free(source->content);
        source->content = src_cont;

        return new_sn;
}

/* makes list link appear at the @position, 
 * @returns next list entry after position
 * The intergrity of the list is retained */
static struct snippet *snippet_split(struct sn_iter split_pos)
{
        size_t itpos = split_pos.position;
        if (itpos == 0)
                return split_pos.curr_sn;

        if (split_pos.curr_sn->content[itpos] == '\0')
                return dlist_next(split_pos.curr_sn);

        struct snippet *before = split_pos.curr_sn;
        struct snippet *after = snippet_cut(before, itpos);
        dlist_append(before, after);
        return after;
}


/* @returns iterator on first newly inserted character */
struct sn_iter sni_insert(struct sn_iter split_pos, struct snippet *other)
{
        struct snippet *after_other = snippet_split(split_pos);
        dlist_insert(after_other, other);
        struct sn_iter on_other = {
                .curr_sn = other,
                .position = 0,
        };
        return on_other;
}


struct sn_iter sni_advance(struct sn_iter iter, size_t offset)
{
        if (sni_cmp(iter, snippet_end()))
                return iter;

        size_t dist_toend = strlen(iter.curr_sn->content);
        offset += iter.position + 1;
        while (offset > dist_toend) {
                if (dlhas_next(iter.curr_sn)) {
                        offset -= dist_toend;        
                        iter.curr_sn = dlist_next(iter.curr_sn);
                        dist_toend = strlen(iter.curr_sn->content);
                } else
                        return snippet_end();
        } 
        iter.position = offset - 1;
        return iter;
}


_Bool sni_cmp(struct sn_iter first, struct sn_iter second)
{
        if (first.curr_sn == NULL || second.curr_sn == NULL)
                return first.curr_sn == second.curr_sn;
        return first.curr_sn == second.curr_sn 
        && first.position == second.position;
}


struct sn_iter snippet_begin(struct snippet *sn)
{
        struct sn_iter result = { 
                .curr_sn = sn,
                .position = 0,
        };
        return result;
}


struct sn_iter snippet_end()
{
        struct sn_iter result = {
                .curr_sn = NULL,
                .position = 0,
        };
        return result;
}
