#ifndef _IR_SCALAR_H_
#define _IR_SCALAR_H_

enum scalar_type {
        scalar_int,
};

struct scalar_var {
        enum scalar_type type;
        union {
                long long int var_int;
                unsigned long long int var_uint;
                long double var_long_double;
        } data;
};

static inline struct scalar_var scalar_create_int(int value)
{
        struct scalar_var result = {
                .type = scalar_int,
                .data.var_int = value,
        };
        return result;
}

#endif /* _IR_SCALAR_H_ */