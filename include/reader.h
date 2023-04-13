#ifndef _READER_H_
#define _READER_H_

#include <stddef.h>

#include <prunit.h>
#include <mc.h>

struct unit_reader {
        
        /* line, begins from start of snippet */
        size_t curr_line;

        struct pru_iter rd_pos;

        struct pru_iter rd_end;

};

struct unit_reader *reader_create(struct pru_iter begin, struct pru_iter end);

/* @returs '\n' on file edge, EOF on stream end */
char_t reader_getc(struct unit_reader *reader);

char_t reader_putc(struct unit_reader *reader, char_t chr);

const char *reader_file_name(const struct unit_reader *reader);

size_t reader_line_number(const struct unit_reader *reader);

void reader_destroy(struct unit_reader *reader);

#endif /* _READER_H_ */
