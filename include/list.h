#ifndef _LIST_H_
#define _LIST_H_

struct dlist_head {
        struct dlist_head *next, *prev;
};

static inline void *dlist_next(void *head)
{
        return ((struct dlist_head*)head)->next;
}

void dlist_free(void *first_head, void (*call_free)(void*));

#endif /* _LIST_H_ */
