#ifndef _IR_SCALAR_H_
#define _IR_SCALAR_H_
#include <stdbool.h>
#include <assert.h>
#include <mc.h>

enum scalar_type {
        scalar_invalid,
        scalar_integer_first,
        s_i1 = scalar_integer_first,
        s_i8,
        s_i16,
        s_i32,
        s_i64,
        scalar_integer_last = s_i64,

        scalar_float_first,

        s_f32 = scalar_float_first,
        s_f64,
        s_f80,

        scalar_float_last = s_f80,
        s_complex,
        s_pointer,
};

#define IR_SCALAR_S_I1_MASK  1ULL
#define IR_SCALAR_S_I8_MASK  ~(uint8_t)0
#define IR_SCALAR_S_I16_MASK ~(uint16_t)0
#define IR_SCALAR_S_I32_MASK ~(uint32_t)0
#define IR_SCALAR_S_I64_MASK ~(uint64_t)0

typedef unsigned long long int u_ir_scalar_t;
typedef long long int i_ir_scalar_t;

/* Scalar Types include:
 * Arithmetic Types: Integer types, Floating types
 * Pointer Types
 * Enumeration Types 
 */
struct ir_scalar {
        enum scalar_type type;
        union {
                u_ir_scalar_t var_int;
                u_ir_scalar_t var_uint;
                long double var_long_double;
        } data;
        _Bool is_signed;
};

static inline _Bool ir_scalar_type_integer(enum scalar_type type)
{
        return (type >= scalar_integer_first && type <= scalar_integer_last);
}

static inline _Bool ir_scalar_is_integer(struct ir_scalar var)
{
        return ir_scalar_type_integer(var.type);
}

static inline _Bool ir_scalar_is_float(struct ir_scalar var)
{
        return (var.type >= scalar_float_first && var.type <= scalar_float_last);
}

static inline _Bool ir_scalar_is_arithmetic(struct ir_scalar var)
{
        return ir_scalar_is_integer(var) || ir_scalar_is_float(var); 
}

static inline _Bool ir_scalar_is_pointer(struct ir_scalar var)
{
        return (var.type == s_pointer);
}

static inline _Bool ir_scalar_type_compatible(struct ir_scalar var1, 
                                              struct ir_scalar var2)
{
        return (var1.type == var2.type);
} 

_Bool ir_scalar_const_cmp(struct ir_scalar var1, struct ir_scalar var2);

mc_status_t ir_scalar_const_cast(struct ir_scalar *var,
                                 enum scalar_type type);

mc_status_t ir_scalar_or(_IN struct ir_scalar var1, 
                         _IN struct ir_scalar var2,
                         _OUT struct ir_scalar *result);

mc_status_t ir_scalar_xor(_IN struct ir_scalar var1, 
                          _IN struct ir_scalar var2,
                          _OUT struct ir_scalar *result);

mc_status_t ir_scalar_and(_IN struct ir_scalar var1, 
                          _IN struct ir_scalar var2,
                          _OUT struct ir_scalar *result);

static inline struct ir_scalar ir_scalar_create_int(int value)
{
        struct ir_scalar result = {
                .type = s_i32,
                .data.var_int = value,
        };
        return result;
}

static inline _Bool ir_scalar_eval_true(struct ir_scalar val)
{
        UNUSED(val);
        assert(false); /* TBD */
        return false;
}

#endif /* _IR_SCALAR_H_ */