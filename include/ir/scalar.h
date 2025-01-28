#ifndef _IR_SCALAR_H_
#define _IR_SCALAR_H_
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
};

#define IR_SCALAR_S_I1_MASK  1ULL
#define IR_SCALAR_S_I8_MASK  ~(uint8_t)0
#define IR_SCALAR_S_I16_MASK ~(uint16_t)0
#define IR_SCALAR_S_I32_MASK ~(uint32_t)0
#define IR_SCALAR_S_I64_MASK ~(uint64_t)0

typedef unsigned long long int u_ir_svar_t;
typedef long long int i_ir_svar_t;

/* Scalar Types include:
 * Arithmetic Types: Integer types, Floating types
 * Pointer Types
 * Enumeration Types 
 */
struct scalar_var {
        enum scalar_type type;
        union {
                i_ir_svar_t var_int;
                u_ir_svar_t var_uint;
                long double var_long_double;
        } data;
        _Bool is_signed;
};

static inline _Bool ir_scalar_type_integer(enum scalar_type type)
{
        return (type >= scalar_integer_first && type <= scalar_integer_last);
}

static inline _Bool ir_scalar_is_integer(struct scalar_var var1)
{
        return ir_scalar_type_integer(var1.type);
}

static inline _Bool ir_scalar_type_compatible(struct scalar_var var1, 
                                              struct scalar_var var2)
{
        return (var1.type == var2.type);
} 

mc_status_t ir_scalar_const_cast(struct scalar_var *var,
                                 enum scalar_type type);

mc_status_t ir_scalar_or(_IN struct scalar_var var1, 
                         _IN struct scalar_var var2,
                         _OUT struct scalar_var *result);

mc_status_t ir_scalar_xor(_IN struct scalar_var var1, 
                          _IN struct scalar_var var2,
                          _OUT struct scalar_var *result);

static inline struct scalar_var ir_scalar_create_int(int value)
{
        struct scalar_var result = {
                .type = s_i32,
                .data.var_int = value,
        };
        return result;
}

#endif /* _IR_SCALAR_H_ */