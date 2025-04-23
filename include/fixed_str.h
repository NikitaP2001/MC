#include <mc.h>

#define FIXED_STR_SIZE 32
typedef file_size_t fstr_sz_t;

struct fixed_str {
        union {
                char fixed[FIXED_STR_SIZE];
                char *dynamic;
        } buf;
        fstr_sz_t len; 
};

static inline 
_Bool
fixed_str_is_fixed(struct fixed_str *f_str)
{
        return (f_str->len < FIXED_STR_SIZE);
}

static inline 
void 
fixed_str_init(struct fixed_str *f_str, const char *str, fstr_sz_t size)
{
        char *buf = NULL;
        if (size < FIXED_STR_SIZE) {
                buf = f_str->buf.fixed;
        } else {
                buf = malloc(size + 1);
                f_str->buf.dynamic = buf;
        }
        memcpy(buf, str, size);
        buf[size] = '\0';
        f_str->len = size;
}

static inline
void
fixed_str_free(struct fixed_str *f_str)
{
        if (!fixed_str_is_fixed(f_str))
                free(f_str->buf.dynamic);
}

static inline char *fixed_str_get(struct fixed_str *f_str)
{
        return fixed_str_is_fixed(f_str)
                ? f_str->buf.fixed : f_str->buf.dynamic;
}