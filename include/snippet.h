#ifndef _SNIPPET_H_
#define _SNIPPET_H_
#include <stddef.h>

#include <list.h>
#include <mc.h>


struct snippet {

        struct dlist_head list;

        char *file_name;
        
        size_t line_number;

        char_t *content;
};

/* @return null in result of alocation error */
struct snippet *snippet_create(const char *file_name);

static inline void snippet_append(struct snippet *position, 
                struct snippet *new_snippet)
{
        dlist_append(position, new_snippet);
}

static inline void snippet_insert(struct snippet *position,
                struct snippet *new_snippet)
{
        dlist_insert(position, new_snippet);
}

enum mc_status snippet_read_file(struct snippet *sn);

void snippet_destroy(struct snippet *sn);

#endif /* _SNIPPET_H_ */
