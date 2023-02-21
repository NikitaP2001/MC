#include <stddef.h>

#include <list.h>


void dlist_free(void *first_head, void (*call_free)(void*))
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
