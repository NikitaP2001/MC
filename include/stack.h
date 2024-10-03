#ifndef _STACK_H_
#define _STACK_H_
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mc.h>

typedef void (*stack_element_free)(void *);

struct stack {
        void *storage;
        size_t size;
        size_t capacity;
        size_t element_size; 
        stack_element_free free;
};

static inline void 
stack_init(struct stack *st, size_t capacity, size_t element_size)
{
        st->storage = calloc(capacity, element_size);
        st->capacity = capacity;
        st->size = 0;
        st->element_size = element_size;
        st->free = NULL;
}

static inline void
stack_set_free(struct stack *st, stack_element_free free)
{
        st->free = free;
}

static inline void stack_resize(struct stack *st)
{
        if (st->size == st->capacity) {
                st->capacity *= 2;
                st->storage = realloc(st->storage, 
                        st->capacity * st->element_size);
        }
}

static inline void 
stack_push(struct stack *st, const void *data)
{
        stack_resize(st);
        const int elem_size = st->element_size;
        uint8_t *dest = (uint8_t *)st->storage + st->size * elem_size;
        memcpy(dest, data, elem_size);
        st->size += 1;
}

static inline _Bool
stack_empty(struct stack *st)
{
        return (st->size == 0);
}

static inline void
stack_top(_IN struct stack *st, _OUT void *result)
{
        const int elem_size = st->element_size;
        const int last_idx = st->size - 1;
        uint8_t *src = (uint8_t *)st->storage + last_idx * elem_size;
        assert(st->size != 0);
        memcpy(result, src, elem_size);
}

static inline void 
stack_pop(struct stack *st)
{
        assert(st->size != 0);
        st->size -= 1;
}

static inline 
void stack_free(struct stack *st)
{
        free(st->storage);
}

static inline void *stack_raw(struct stack *st)
{
        return st->storage;
}

#endif /* _STACK_H_ */