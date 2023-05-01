#ifndef _TTOOL_H_
#define _TTOOL_H_
#include <ctype.h>
/* Tools local for test suite */

/* from content determines real source file content length */
size_t cont_len(char_t *content);

/* in preprocessing newline is not treated 
 * as whitespace */
static inline _Bool is_white_space(int chr)
{
        return isspace(chr) && chr != '\n';
}

_Bool wordcmp(char *str1, char *str2);

_Bool write_file(const char *file_name, const char *content, size_t length);

#endif /* _TTOOL_H_ */