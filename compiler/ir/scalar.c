#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#include <token.h>
#include <ir/scalar.h>

#define IR_SCALAR_S_MSB_MASK(type) (1ULL << (sizeof(type) * CHAR_BIT - 1))
#define IR_SCALAR_S_I8_MSB_MASK  IR_SCALAR_S_MSB_MASK(uint8_t)
#define IR_SCALAR_S_I16_MSB_MASK IR_SCALAR_S_MSB_MASK(uint16_t)
#define IR_SCALAR_S_I32_MSB_MASK IR_SCALAR_S_MSB_MASK(uint32_t)
#define IR_SCALAR_S_I64_MSB_MASK IR_SCALAR_S_MSB_MASK(uint64_t)

static void ir_scalar_const_sign_ext(struct ir_scalar *value)
{
        assert(ir_scalar_is_integer(*value));
        _Bool is_signed = value->is_signed;
        u_ir_scalar_t *u_val = &value->data.var_uint;
        switch (value->type) {
                case s_i1:
                        /* _Bool should be unsigned */
                        assert(!is_signed);
                        *u_val &= IR_SCALAR_S_I1_MASK;
                        break;
                case s_i8:
                        if (is_signed && (IR_SCALAR_S_I8_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_scalar_t)IR_SCALAR_S_I8_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I8_MASK;
                        break;

                case s_i16:
                        if (is_signed && (IR_SCALAR_S_I16_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_scalar_t)IR_SCALAR_S_I16_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I16_MASK;
                        break;

                case s_i32:
                        if (is_signed && (IR_SCALAR_S_I32_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_scalar_t)IR_SCALAR_S_I32_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I32_MASK;
                        break;

                case s_i64:
                        if (is_signed && (IR_SCALAR_S_I64_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_scalar_t)IR_SCALAR_S_I64_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I64_MASK;
                        break;

                default:
                        MC_DBG(MC_CRIT, "unexpected scalar type");
        }
}

mc_status_t ir_scalar_const_cast(struct ir_scalar *var,
                                 enum scalar_type type)
{ 
        mc_status_t status = MC_FAIL;
        /* TODO: may we need handle other scalar types */
        if (ir_scalar_is_integer(*var) && ir_scalar_type_integer(type)) {
                var->type = type;
                ir_scalar_const_sign_ext(var);
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_scalar_add(_IN struct ir_scalar var1, 
                         _IN struct ir_scalar var2,
                         _OUT struct ir_scalar *result)
{
        if (!ir_scalar_type_compatible(var1, var2) 
                || !ir_scalar_type_compatible(var1, *result))
                return MC_FAIL;
        if (ir_scalar_is_integer(var1)) {
                if (var1.is_signed) {
                        i_ir_scalar_t sum = (i_ir_scalar_t)var1.data.var_int 
                                + (i_ir_scalar_t)var2.data.var_int;
                        /* check overflow */
                        switch (var1.type) {
                                case s_i8:
                                        if ((sum > INT8_MAX) || (sum < INT8_MIN))
                                                return MC_FAIL;
                                        break;
                                case s_i16:
                                        if ((sum > INT16_MAX) || (sum < INT16_MIN))
                                                return MC_FAIL;
                                        break;
                                case s_i32:
                                        if ((sum > INT32_MAX) || (sum < INT32_MIN))
                                                return MC_FAIL;
                                        break;
                                case s_i64:
                                        if ((sum > INT64_MAX) || (sum < INT64_MIN))
                                                return MC_FAIL;
                                        break;
                                default:
                                        MC_DBG(MC_CRIT, "unexpected scalar type");
                        }
                        *result = ir_scalar_create_int(var1.type, sum);
                } else {
                        u_ir_scalar_t sum = var1.data.var_uint + var2.data.var_uint;
                        /* check overflow */
                        switch (var1.type) {
                                case s_i8:
                                        if (sum > UINT8_MAX)
                                                return MC_FAIL;
                                        break;
                                case s_i16:
                                        if (sum > UINT16_MAX)
                                                return MC_FAIL;
                                        break;
                                case s_i32:
                                        if (sum > UINT32_MAX)
                                                return MC_FAIL;
                                        break;
                                case s_i64:
                                        /* no overflow for unsigned 64-bit integer */
                                        break;
                                default:
                                        MC_DBG(MC_CRIT, "unexpected scalar type");
                        }
                        *result = ir_scalar_create_uint(var1.type, sum);
                }
                ir_scalar_const_sign_ext(result); 
                return MC_OK; 
        }
        return MC_FAIL; 
}

mc_status_t ir_scalar_or(_IN struct ir_scalar var1, 
                         _IN struct ir_scalar var2,
                         _OUT struct ir_scalar *result)
{
        if (!ir_scalar_type_compatible(var1, var2) 
                || !ir_scalar_type_compatible(var1, *result))
                return MC_FAIL;
        result->data.var_uint = var1.data.var_uint | var2.data.var_uint;
        ir_scalar_const_sign_ext(result); 
        return MC_OK; 
}

mc_status_t ir_scalar_xor(_IN struct ir_scalar var1, 
                          _IN struct ir_scalar var2,
                          _OUT struct ir_scalar *result)
{
        if (!ir_scalar_type_compatible(var1, var2) 
                || !ir_scalar_type_compatible(var1, *result))
                return MC_FAIL;
        result->data.var_uint = var1.data.var_uint ^ var2.data.var_uint;
        ir_scalar_const_sign_ext(result); 
        return MC_OK; 
}

mc_status_t ir_scalar_and(_IN struct ir_scalar var1, 
                          _IN struct ir_scalar var2,
                          _OUT struct ir_scalar *result)
{
        if (!ir_scalar_type_compatible(var1, var2) 
                || !ir_scalar_type_compatible(var1, *result))
                return MC_FAIL;
        result->data.var_uint = var1.data.var_uint & var2.data.var_uint;
        ir_scalar_const_sign_ext(result); 
        return MC_OK;
}

_Bool ir_scalar_const_cmp(struct ir_scalar var1, 
                          struct ir_scalar var2)
{
        _Bool equal = false;
        assert(ir_scalar_type_compatible(var1, var2));
        if (ir_scalar_is_integer(var1)) {
                if (var1.is_signed)
                        equal = (var1.data.var_int == var2.data.var_int);
                else
                        equal = (var1.data.var_uint == var2.data.var_uint);
        } else if (ir_scalar_is_float(var1)) {
                equal = (var1.data.var_long_double 
                        == var2.data.var_long_double);
        }
        return equal;
}

static inline 
enum scalar_type 
ir_scalar_const_type_to_scalar(enum constant_type type)
{
        switch (type) {
                case const_int:
                        return s_i32;
                case const_uint:
                        return s_i32;
                case const_long_int:
                        return s_i64;
                case const_ulong_int:
                        return s_i64;
                case const_long_long_int:
                        return s_i64;
                case const_ulong_long_int:
                        return s_i64;
                case const_float:
                        return s_f32;
                case const_double:
                        return s_f64;
                case const_long_double:
                        return s_f80;
                default:
                        MC_DBG(MC_CRIT, "unexpected constant type");
                        return scalar_absent;
        }
}

void ir_scalar_from_token(struct ir_scalar *scalar, struct token *tok)
{
        struct constant_value val = token_constant(tok);
        enum constant_type type = val.type;
        scalar->type = ir_scalar_const_type_to_scalar(type);
        if (token_const_is_integer(type)) {
                if (token_const_is_signed(type)) {
                        scalar->data.var_int = val.data.var_int;
                        scalar->is_signed = true;
                } else {
                        scalar->data.var_uint = val.data.var_uint;
                        scalar->is_signed = false;
                }
        } else if (token_const_is_float(type)) {
                scalar->data.var_long_double = val.data.var_long_double;
                scalar->is_signed = true;
        } else {
                MC_DBG(MC_CRIT, "unexpected constant type");
        }
}