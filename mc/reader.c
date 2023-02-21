#include <stdlib.h>
#include <stdio.h>

#include <reader.h>

static struct snippet *skip_empty(struct unit_reader *reader)
{
        struct snippet *head = reader->curr_snippet;
        char_t *read_ptr = head->content + reader->read_pos;

        while (*read_ptr == '\0' && head) {
                head = dlist_next(reader->curr_snippet);
                if (head != NULL) {
                        reader->curr_snippet = head;
                        read_ptr = head->content;
                        reader->read_pos = 0;
                        reader->curr_line = 0;
                }
        }

        return head;
}

/* @return pointer to next non-zero char, or NULL if none left */
static inline char_t *get_read_ptr(struct unit_reader *reader)
{
        struct snippet *head = skip_empty(reader);
        return head == NULL ? NULL : head->content + reader->read_pos;
}


char_t reader_getc(struct unit_reader *reader)
{
        char_t next_chr = EOF;
        struct snippet *start_snippet = reader->curr_snippet;

        char_t *read_ptr = get_read_ptr(reader);
        if (read_ptr != NULL) {
                if (start_snippet == reader->curr_snippet) {
                        next_chr = *read_ptr;
                        reader->read_pos += 1;
                        if (next_chr == '\n')
                                reader->curr_line += 1;
                } else
                        next_chr = '\n';
        }

        return next_chr;
}


const char *reader_file_name(const struct unit_reader *reader)
{
        return reader->curr_snippet->file_name;
}


size_t reader_line_number(const struct unit_reader *reader)
{
        size_t snippet_line = reader->curr_snippet->line_number;
        return snippet_line + reader->curr_line;
}


void reader_destroy(struct unit_reader *reader)
{
        free(reader);
}
