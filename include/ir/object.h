#ifndef _IR_OBJECT_H_
#define _IR_OBJECT_H_
#include <ir/scalar.h>
#include <ir/value.h>

enum ir_object_type {
        /* this is default abstract state */
        ir_object_unspecified,
        ir_object_scalar,
        ir_object_array,
        ir_object_struct,
        ir_object_union,
};

struct ir_object {
        struct ir_value val;
        enum ir_object_type type;
        union {
                struct ir_scalar scalar;
        } var;
        struct pt_node *node;
        /* leads to starting evaluation block, is NULL is const eval */
        struct basic_block *eval;
        /*
        _Bool is_modifiable;
        _Bool is_const;
        */
};

struct ir_scalar;

/* specify, if any ir-code was, starting at @start were used for @val 
 * creation or @val is const eval */
static inline void ir_obj_set_eval(struct ir_object *val, struct basic_block *start)
{
        val->eval = start;
}

static inline 
struct basic_block *ir_obj_get_eval(struct ir_object *val)
{
        return val->eval;
}

/* is value compile time evaluable (example: const expr result) */
static inline _Bool ir_obj_const_eval(struct ir_object *val)
{
        return (val->eval == NULL);
}

static inline 
struct ir_object *
ir_obj_get(struct ir_value *val)
{
        assert(val->type == ir_value_object);
        return container_of(val, struct ir_object, val);
}

static inline _Bool ir_obj_is_scalar(struct ir_object *val)
{
        return (val->type == ir_object_scalar);
}

static inline struct ir_scalar ir_obj_scalar_get(struct ir_object *val)
{
        assert(ir_obj_is_scalar(val));
        return val->var.scalar;
}

static inline void ir_obj_scalar_set(struct ir_object *val, struct ir_scalar scalar)
{
        assert(val->type == ir_object_unspecified || val->type == ir_object_scalar);
        val->type = ir_object_scalar;
        val->var.scalar = scalar;
}

/* Integer and floating types are collectively called arithmetic types. */
static inline _Bool ir_obj_is_arithmetic(struct ir_object *val)
{
        assert(ir_obj_is_scalar(val));
        return ir_scalar_is_arithmetic(val->var.scalar);
}

static inline _Bool ir_obj_is_integer(struct ir_object *val)
{
        return (ir_obj_is_arithmetic(val) 
                && ir_scalar_is_integer(val->var.scalar));
}

static inline 
_Bool ir_obj_compat_struct_or_union(struct ir_object *val1, 
                                   struct ir_object *val2)
{
        UNUSED(val1);
        UNUSED(val2);
        assert(false);
        return false;
}

static inline 
_Bool ir_obj_is_pointer(struct ir_object *val)
{
        return ir_obj_is_scalar(val) 
                && ir_scalar_is_pointer(ir_obj_scalar_get(val));
}

/* pointer to incoplete type */
static inline 
_Bool ir_obj_is_ptr_incompl(struct ir_object *val)
{
        UNUSED(val);
        assert(false);
        return false;
}

/* pointer to object type */
static inline 
_Bool ir_obj_is_ptr_obj(struct ir_object *val)
{
        assert(false);
        return ir_obj_is_pointer(val);
}

static inline 
_Bool ir_obj_is_nullptr(struct ir_object *val)
{
        UNUSED(val);
        assert(false);
        return false;
}

static inline 
_Bool ir_obj_is_ptr_void(struct ir_object *val)
{
        UNUSED(val);
        assert(false);
        return false;
}

/* pointers to qualified or unqualified versions of compatible types; */
static inline 
_Bool ir_obj_ptr_compat(struct ir_object *val1, 
                        struct ir_object *val2)
{
        UNUSED(val1);
        UNUSED(val2);
        assert(false);
        return false;
}

struct ir_object *ir_obj_create(const char *prefix, uint32_t index);

struct ir_value *ir_obj_value_get(struct ir_object *val);

mc_status_t ir_obj_const_move(struct ir_object *source, struct ir_object *result);

void ir_obj_sext(struct basic_block *bb, struct ir_object *src, 
                 struct ir_object *dest, enum scalar_type type);

void ir_obj_zext(struct basic_block *bb, struct ir_object *src, 
                 struct ir_object *dest, enum scalar_type type);
                        
mc_status_t ir_obj_or(struct basic_block *bb, struct ir_object *val1, 
                      struct ir_object *val2, struct ir_object *result);

mc_status_t ir_obj_xor(struct basic_block *bb, struct ir_object *val1, 
                       struct ir_object *val2, struct ir_object *result);

mc_status_t ir_obj_and(struct basic_block *bb, struct ir_object *val1, 
                       struct ir_object *val2, struct ir_object *result);

mc_status_t ir_obj_cmp(struct basic_block *bb, enum ir_ins_type type, 
        struct ir_object *val1, struct ir_object *val2, struct ir_object *result);

mc_status_t ir_obj_const_set(struct ir_object *dest, struct ir_scalar value);

static inline 
mc_status_t ir_obj_scalar_const_cast(struct ir_object *val, 
                                    enum scalar_type type)
{
        return ir_scalar_const_cast(&val->var.scalar, type);
}

/* to @bb add instructions to move src value to dest */
mc_status_t ir_obj_move(struct basic_block *bb, struct ir_object *src, 
        struct ir_object *dest);

static inline _Bool ir_obj_eval_true(struct ir_object *val)
{
        assert(ir_obj_is_scalar(val));
        return ir_scalar_eval_true(val->var.scalar);
}

#endif /* _IR_OBJECT_H_ */