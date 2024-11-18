#ifndef _IR_VALUE_H_
#define _IR_VALUE_H_
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <tools/hashtable.h>
#include <ir/scalar.h>
#include <mc.h>
#include <vector.h>

enum ir_value_type {
        ir_value_basic_block,
        ir_value_lvalue,
};
#define IR_VALUE_NAME_MAX 32
#define IR_VALUE_USE_COUNT 5

struct value {
        char name[IR_VALUE_NAME_MAX];
        enum ir_value_type type;
        struct ir_ins_use **uses;
        size_t use_count;
        size_t use_capacity;

        hash_key_t hash;
        struct hlist_entry hlist;
};

void ir_value_add_use(struct value *val, struct ir_ins_use *use);

static inline 
struct value *
ir_value_hlist_entry(struct hlist_entry *node)
{
        return container_of(node, struct value, hlist);
}

hash_key_t ir_value_hash(struct hlist_entry *node);

void ir_value_free(struct hlist_entry *node);

#define IR_BASIC_BLOCK_INS_COUNT 5

struct instruction;

struct basic_block {
        struct value val;

        struct vector ins_buf;
};

static inline 
struct basic_block *
ir_value_bb_get(struct value *val)
{
        assert(val->type == ir_value_basic_block);
        return container_of(val, struct basic_block, val);
}

struct basic_block *ir_bb_create(const char *prefix, uint32_t index);

static inline _Bool ir_bb_empty(struct basic_block *bb)
{
        return vector_empty(&bb->ins_buf);
}

/* basic block is complete - ending with control flow ins */
_Bool ir_bb_is_complete(struct basic_block *block);

size_t ir_bb_ins_count(struct basic_block *block);

struct instruction *ir_bb_ins_get(struct basic_block *block, size_t ins_num);

static inline 
struct instruction *
ir_bb_ins_last(struct basic_block *block)
{
        return ir_bb_ins_get(block, ir_bb_ins_count(block) - 1);
}

static inline 
struct value *
ir_bb_value(struct basic_block *block)
{
        return &block->val;
}

enum lvalue_type {
        /* this is default abstract state */
        lvalue_unspecified,
        lvalue_scalar,
        lvalue_void,
        lvalue_ptr,
};

struct lvalue {
        struct value val;
        enum lvalue_type type;
        union {
                struct scalar_var scalar;
        } var;
        struct pt_node *node;
        /* leads to starting evaluation block, is NULL is const eval */
        struct basic_block *eval;
};

void ir_bb_ctf(struct basic_block *prev, struct basic_block *first, 
               struct basic_block *second, struct lvalue *condition);

static inline void ir_bb_ctf_uncond(struct basic_block *before, 
                                    struct basic_block *after)
{
        ir_bb_ctf(before, after, NULL, NULL);
}

/* specify, if any ir-code was, starting at @start were used for @val 
 * creation or @val is const eval */
static inline void ir_lvalue_set_eval(struct lvalue *val, struct basic_block *start)
{
        val->eval = start;
}

static inline 
struct basic_block *ir_lvalue_get_eval(struct lvalue *val)
{
        return val->eval;
}

/* is value compile time evaluable (example: const expr result) */
static inline _Bool ir_lvalue_const_eval(struct lvalue *val)
{
        return (val->eval == NULL);
}

static inline 
struct lvalue *
ir_lvalue_get(struct value *val)
{
        assert(val->type == ir_value_lvalue);
        return container_of(val, struct lvalue, val);
}

static inline _Bool ir_lvalue_is_arithmetic(struct lvalue *val)
{
        UNUSED(val);
        assert(false);
        return (val->type == lvalue_scalar);
}

static inline 
_Bool ir_lvalue_compat_struct_or_union(struct lvalue *val1, 
                                       struct lvalue *val2)
{
        UNUSED(val1);
        UNUSED(val2);
        assert(false);
        return false;
}

static inline 
_Bool ir_lvalue_is_pointer(struct lvalue *val)
{
        return (val->type == lvalue_ptr);
}

/* pointer to object or incoplete type */
static inline 
_Bool ir_lvalue_is_ptr_obj(struct lvalue *val)
{
        assert(false);
        return ir_lvalue_is_pointer(val);
}

static inline 
_Bool ir_lvalue_is_nullptr(struct lvalue *val)
{
        UNUSED(val);
        assert(false);
        return false;
}

static inline 
_Bool ir_lvalue_is_ptr_void(struct lvalue *val)
{
        UNUSED(val);
        assert(false);
        return false;
}

/* pointers to qualified or unqualified versions of compatible types; */
static inline 
_Bool ir_lvalue_ptr_compat(struct lvalue *val1, 
                           struct lvalue *val2)
{
        UNUSED(val1);
        UNUSED(val2);
        assert(false);
        return false;
}

struct lvalue *ir_lvalue_create(const char *prefix, uint32_t index);

mc_status_t ir_lvalue_const_move(struct lvalue *source, struct lvalue *result);

/* to @bb add instructions to move src value to dest */
mc_status_t  ir_lvalue_move(struct basic_block *bb, struct lvalue *src, 
        struct lvalue *dest);

_Bool ir_scalar_eval_true(struct lvalue *val)
{
        assert(val->type == lvalue_scalar);
        assert(false); /* TBD */
        return false;
}

#endif /* _IR_VALUE_H_ */