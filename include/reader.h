#ifndef _READER_H_
#define _READER_H_

#include <stddef.h>

#include <snippet.h>

struct unit_reader {
        
        struct snippet *curr_snippet;     

        /* line, begins from start of snippet */
        size_t curr_line;

        /* offset within snippet content array */
        size_t read_pos;

};

/* @returs '\n' on file edge, EOF on stream end */
char_t reader_getc(struct unit_reader *reader);

char_t reader_putc(struct unit_reader *reader, char_t chr);

const char *reader_file_name(const struct unit_reader *reader);

size_t reader_line_number(const struct unit_reader *reader);

void reader_destroy(struct unit_reader *reader);

#endif /* _READER_H_ */
