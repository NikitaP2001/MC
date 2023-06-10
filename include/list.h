#ifndef _LIST_H_
#define _LIST_H_
#include <stddef.h>

struct list_head {
        struct list_head *next, *prev;
};

static inline _Bool list_has_next(void *head)
{
        return ((struct list_head*)head)->next != NULL;
}

static inline void *list_next(void *head)
{
        return ((struct list_head*)head)->next;
}

#define LIST_FOREACH_ENTRY(first) \
for (struct list_head *entry = (struct list_head*)first; \
entry != NULL; entry = list_next(entry))

static inline void *list_prev(void *head)
{
        return ((struct list_head*)head)->prev;
}

void list_append(void *position, void *new_head);

void list_insert(void *position, void *new_head);

void list_destroy(void *first_head, void (*call_free)(void*));

#endif /* _LIST_H_ */
