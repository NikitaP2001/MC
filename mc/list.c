#include <stddef.h>

#include <list.h>


static inline void list_link(struct list_head *before, 
                struct list_head *middle, struct list_head *after)
{
        if (before != NULL)
                before->next = middle;
        middle->prev = before;
        middle->next = after;
        if (after != NULL)
                after->prev = middle;
}


void list_append(void *position, void *new_head)
{
        struct list_head* before = position;
        struct list_head* middle = new_head;
        struct list_head* after = before->next;
        list_link(before, middle, after);
}


void list_insert(void *position, void *new_head)
{
        struct list_head* after = position;
        struct list_head* before = after->prev;
        struct list_head* middle = new_head;
        list_link(before, middle, after);
}


void list_destroy(void *first_head, void (*call_free)(void*))
{
        struct list_head *head = first_head;
        struct list_head *next = NULL;

        if (head->prev != NULL)
                head->prev->next = NULL;

        while (head != NULL) {
                next = head->next;
                call_free(head);
                head = next; 
        }
}
