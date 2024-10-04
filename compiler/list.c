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

static inline 
void _list_replace_range(struct list_head *r1_f, struct list_head *r1_l,
                        struct list_head *r2_f, struct list_head *r2_l)
{
        struct list_head *r1_pre = list_prev(r1_f);
        struct list_head *r1_post = list_next(r1_l);
        if (r1_pre)
                list_setnext(r1_pre, r2_f);
        list_setprev(r1_f, NULL);
        list_setprev(r2_f, r1_pre);
        if (r1_post)
                list_setprev(r1_post, r2_l);
        list_setnext(r1_l, NULL);
        list_setnext(r2_l, r1_post);
}

void list_replace_range(void *range1_first, void *range1_last,
                        void *range2_first, void *range2_last)
{
        _list_replace_range((struct list_head *)range1_first,
                           (struct list_head *)range1_last,
                           (struct list_head *)range2_first,
                           (struct list_head *)range2_last);
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

void list_destroy_range(void *first, void *last, list_free call_free)
{
        struct list_head *head = (struct list_head*)first;
        struct list_head *pre_first = head->prev;

        while (head != last) {
                struct list_head *prev = head;
                head = list_next(prev);
                call_free(prev);      
        }
        struct list_head *post_last = head->next;
        if (pre_first != NULL)
                pre_first->next = post_last;
        if (post_last != NULL)
                post_last->prev = pre_first;
        call_free(head);
}