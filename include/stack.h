#ifndef _STACK_H_
#define _STACK_H_
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mc.h>
#include <vector.h>

struct stack {
        struct vector vec;
};

static inline void 
stack_init(struct stack *st, size_t capacity, size_t element_size)
{
        vector_init(&st->vec, capacity, element_size);
}

static inline void 
stack_push(struct stack *st, const void *data)
{
        vector_push_back(&st->vec, data);
}

static inline _Bool
stack_empty(struct stack *st)
{
        return vector_empty(&st->vec);
}

static inline void
stack_top(_IN struct stack *st, _OUT void *result)
{
        uint8_t elem_size = st->vec.element_size;
        uint8_t *src = vector_last(&st->vec);
        memcpy(result, src, elem_size);
}

static inline void 
stack_pop(struct stack *st)
{
        vector_pop_back(&st->vec);
}

static inline 
void stack_free(struct stack *st)
{
        vector_free(&st->vec);
}

static inline void *stack_raw(struct stack *st)
{
        return vector_raw(&st->vec);
}

#endif /* _STACK_H_ */