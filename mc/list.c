#include <stddef.h>

#include <list.h>




static inline void dlink_link(struct dlist_head *before, 
                struct dlist_head *middle, struct dlist_head *after)
{
        if (before != NULL)
                before->next = middle;
        middle->prev = before;
        middle->next = after;
        if (after != NULL)
                after->prev = middle;
}


void dlist_append(void *position, void *new_head)
{
        struct dlist_head* before = position;
        struct dlist_head* middle = new_head;
        struct dlist_head* after = before->next;
        dlink_link(before, middle, after);
}


void dlist_insert(void *position, void *new_head)
{
        struct dlist_head* after = position;
        struct dlist_head* before = after->prev;
        struct dlist_head* middle = new_head;
        dlink_link(before, middle, after);
}


void dlist_destroy(void *first_head, void (*call_free)(void*))
{
        struct dlist_head *head = first_head;
        struct dlist_head *next = NULL;

        if (head->prev != NULL)
                head->prev->next = NULL;

        while (head != NULL) {
                next = head->next;
                call_free(head);
                head = next; 
        }
}
