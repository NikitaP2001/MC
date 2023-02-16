#ifndef _READER_
#define _READER_

#include <stddef.h>

#include <snippet.h>

struct unit_reader {
        struct snippet *curr_snippet;     

        /* line, begins from start of snippet */
        size_t curr_line;

        /* offset within snippet content array */
        size_t read_pos;

};

char_t reader_getc(struct unit_reader *reader);

char_t reader_putc(struct unit_reader *reader, char_t chr);

/* if @ptr_line is NULL do nothing, except return 
 * next line length
 * @returns number of chars in line to read */
int reader_getline(struct unit_reader *reader, char_t *ptr_line);

const char *reader_file_name(const struct unit_reader *reader);

size_t reader_line_number(const struct unit_reader *reader);

#endif /* _READER_ */
