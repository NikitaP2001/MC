#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#include <ir/scalar.h>

#define IR_SCALAR_S_MSB_MASK(type) (1ULL << (sizeof(type) * CHAR_BIT - 1))
#define IR_SCALAR_S_I8_MSB_MASK  IR_SCALAR_S_MSB_MASK(uint8_t)
#define IR_SCALAR_S_I16_MSB_MASK IR_SCALAR_S_MSB_MASK(uint16_t)
#define IR_SCALAR_S_I32_MSB_MASK IR_SCALAR_S_MSB_MASK(uint32_t)
#define IR_SCALAR_S_I64_MSB_MASK IR_SCALAR_S_MSB_MASK(uint64_t)

static void ir_scalar_const_sign_ext(struct scalar_var *value)
{
        assert(ir_scalar_is_integer(*value));
        _Bool is_signed = value->is_signed;
        u_ir_svar_t *u_val = &value->data.var_uint;
        switch (value->type) {
                case s_i1:
                        /* _Bool should be unsigned */
                        assert(!is_signed);
                        *u_val &= IR_SCALAR_S_I1_MASK;
                        break;
                case s_i8:
                        if (is_signed && (IR_SCALAR_S_I8_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_svar_t)IR_SCALAR_S_I8_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I8_MASK;
                        break;

                case s_i16:
                        if (is_signed && (IR_SCALAR_S_I16_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_svar_t)IR_SCALAR_S_I16_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I16_MASK;
                        break;

                case s_i32:
                        if (is_signed && (IR_SCALAR_S_I32_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_svar_t)IR_SCALAR_S_I32_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I32_MASK;
                        break;

                case s_i64:
                        if (is_signed && (IR_SCALAR_S_I64_MSB_MASK & *u_val)) {
                                *u_val |= ~(u_ir_svar_t)IR_SCALAR_S_I64_MASK;
                        } else
                                *u_val &= IR_SCALAR_S_I64_MASK;
                        break;

                default:
                        MC_DBG(MC_CRIT, "unexpected scalar type");
        }
}

mc_status_t ir_scalar_const_cast(struct scalar_var *var,
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


mc_status_t ir_scalar_or(_IN struct scalar_var var1, 
                         _IN struct scalar_var var2,
                         _OUT struct scalar_var *result)
{
        if (!ir_scalar_type_compatible(var1, var2) 
                || !ir_scalar_type_compatible(var1, *result))
                return MC_FAIL;
        result->data.var_uint = var1.data.var_uint | var2.data.var_uint;
        ir_scalar_const_sign_ext(result); 
        return MC_OK; 
}

mc_status_t ir_scalar_xor(_IN struct scalar_var var1, 
                          _IN struct scalar_var var2,
                          _OUT struct scalar_var *result)
{
        if (!ir_scalar_type_compatible(var1, var2) 
                || !ir_scalar_type_compatible(var1, *result))
                return MC_FAIL;
        result->data.var_uint = var1.data.var_uint ^ var2.data.var_uint;
        ir_scalar_const_sign_ext(result); 
        return MC_OK; 
}

mc_status_t ir_scalar_and(_IN struct scalar_var var1, 
                          _IN struct scalar_var var2,
                          _OUT struct scalar_var *result)
{
        if (!ir_scalar_type_compatible(var1, var2) 
                || !ir_scalar_type_compatible(var1, *result))
                return MC_FAIL;
        result->data.var_uint = var1.data.var_uint & var2.data.var_uint;
        ir_scalar_const_sign_ext(result); 
        return MC_OK;
}

_Bool ir_scalar_const_cmp(struct scalar_var var1, 
                          struct scalar_var var2)
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