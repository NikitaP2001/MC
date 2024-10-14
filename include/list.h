#ifndef _LIST_H_
#define _LIST_H_
#include <stddef.h>

struct list_head {
        struct list_head *next, *prev;
};

static inline _Bool list_has_next(const void *head)
{
        return ((struct list_head*)head)->next != NULL;
}

static inline _Bool list_has_prev(const void *head)
{
        return ((struct list_head*)head)->prev != NULL;
}

static inline void *list_next(const void *head)
{
        return ((struct list_head*)head)->next;
}

static inline void list_setnext(void *head, void *next)
{
        ((struct list_head*)head)->next = (struct list_head*)next;
}

#define LIST_FOREACH_ENTRY(first) \
for (struct list_head *entry = (struct list_head*)first; \
entry != NULL; entry = list_next(entry))

static inline void *list_prev(void *head)
{
        return ((struct list_head*)head)->prev;
}

static inline void list_setprev(void *head, void *prev)
{
        ((struct list_head*)head)->prev = (struct list_head*)prev;
}

void list_append(void *position, void *new_head);

void list_insert(void *position, void *new_head);

/* note: it did not preserve the integrity of the list, 
 * containing second range, if any  */
void list_replace_range(void *range1_first, void *range1_last,
                        void *range2_first, void *range2_last);

typedef void (*list_free)(void*);

void list_destroy(void *first_head, list_free call_free);

/* range is destructed used @call_free, the integrity of
 * the list, containing range is saved */
void list_destroy_range(void *first, void *last, list_free call_free);

#endif /* _LIST_H_ */
