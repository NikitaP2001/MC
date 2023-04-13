#ifndef _LIST_H_
#define _LIST_H_

struct dlist_head {
        struct dlist_head *next, *prev;
};

static inline _Bool dlhas_next(void *head)
{
        return ((struct dlist_head*)head)->next != NULL;
}

static inline void *dlist_next(void *head)
{
        return ((struct dlist_head*)head)->next;
}

void dlist_append(void *position, void *new_head);

void dlist_insert(void *position, void *new_head);

void dlist_del(void *entry);

void dlist_destroy(void *first_head, void (*call_free)(void*));

#endif /* _LIST_H_ */
