#include <stdlib.h>
#include <stdio.h>

#include <reader.h>


struct unit_reader *reader_create(struct pru_iter begin, struct pru_iter end)
{
        struct unit_reader *result = malloc(sizeof(struct unit_reader));
        if (result != NULL) {
                result->curr_line = 0;
                result->rd_pos = begin; 
                result->rd_end = end;
                for (size_t i = 0; i <= begin.snip_it.position; i++)
                        if (begin.snip_it.curr_sn->content[i] == '\n')
                                result->curr_line += 1;
        }
        return result;
}

static inline char_t reader_pri_getc(struct pru_iter pos)
{
        struct snippet *curr = pos.snip_it.curr_sn;
        size_t rd_pos = pos.snip_it.position;
        return curr->content[rd_pos];
}

char_t reader_getc(struct unit_reader *reader)
{
        char_t rd_char = EOF;
        if (!pri_cmp(reader->rd_pos, reader->rd_pos)) {
                rd_char = reader_pri_getc(reader->rd_pos);
                pri_advance(reader->rd_pos, 1);
        }
        return rd_char;
}


const char *reader_file_name(const struct unit_reader *reader)
{
        return reader->rd_pos.snip_it.curr_sn->file_name;
}


size_t reader_line_number(const struct unit_reader *reader)
{
        size_t snippet_line = reader->rd_pos.snip_it.curr_sn->line_number;
        return snippet_line + reader->curr_line;
}


void reader_destroy(struct unit_reader *reader)
{
        free(reader);
}
