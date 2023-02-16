
typedef struct double_lined_list {
        struct double_lined_list *next, *prev;
} dlist_head;

static inline void *dlist_next(void *head)
{
        return ((dlist_head*)head)->next;
}
