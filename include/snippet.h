#include <stddef.h>

#include <linked_list.h>

typedef char char_t;

struct snippet {

        dlist_head list;

        const char *file_name;
        
        size_t line_number;

        char_t *content;
};

struct snippet *snippet_create(const char *file_name);

_Bool snippet_read_file(struct snippet *sn);

void snippet_destroy(struct snippet *sn);
