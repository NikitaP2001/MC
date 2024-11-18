#ifndef _VECTOR_H_
#define _VECTOR_H_
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mc.h>

struct vector {
        void *storage;
        size_t size;
        size_t capacity;
        size_t element_size; 
};

static inline void 
vector_init(struct vector *vec, size_t capacity, size_t element_size)
{
        vec->storage = calloc(capacity, element_size);
        vec->capacity = capacity;
        vec->size = 0;
        vec->element_size = element_size;
}

static inline void vector_resize(struct vector *vec)
{
        if (vec->size == vec->capacity) {
                vec->capacity *= 2;
                vec->storage = realloc(vec->storage, 
                        vec->capacity * vec->element_size);
        }
}

static inline void 
vector_push_back(struct vector *vec, const void *data)
{
        vector_resize(vec);
        const int elem_size = vec->element_size;
        uint8_t *dest = (uint8_t *)vec->storage + vec->size * elem_size;
        memcpy(dest, data, elem_size);
        vec->size += 1;
}

static inline _Bool
vector_size(struct vector *vec) 
{
        return vec->size;
}

static inline _Bool
vector_empty(struct vector *vec) 
{
        return (vector_size(vec) == 0);
}

static inline void*
vector_get(_IN struct vector *vec, _IN size_t index)
{
        assert(index <= vec->size);
        const int elem_size = vec->element_size;
        uint8_t *src = (uint8_t *)vec->storage + index * elem_size;
        assert(vec->size != 0);
        return src;
}

static inline void*
vector_last(_IN struct vector *vec)
{
        return vector_get(vec, vec->size - 1);
}

static inline void 
vector_pop_back(struct vector *vec)
{
        assert(vec->size != 0);
        vec->size -= 1;
}

static inline 
void vector_free(struct vector *vec)
{
        free(vec->storage);
}

static inline void *vector_raw(struct vector *vec)
{
        return vec->storage;
}

#endif /* _VECTOR_H_ */