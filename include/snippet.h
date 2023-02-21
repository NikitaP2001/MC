#ifndef _SNIPPET_H_
#define _SNIPPET_H_
#include <stddef.h>

#include <list.h>

typedef char char_t;

struct snippet {

        struct dlist_head list;

        char *file_name;
        
        size_t line_number;

        char_t *content;
};

struct snippet *snippet_create(const char *file_name);

_Bool snippet_read_file(struct snippet *sn);

void snippet_destroy(struct snippet *sn);

#endif /* _SNIPPET_H_ */
