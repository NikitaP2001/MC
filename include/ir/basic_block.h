#ifndef _IR_BASIC_BLOCK_H_
#define _IR_BASIC_BLOCK_H_
#include <ir/value.h>
#include <ir/object.h>

#define IR_BASIC_BLOCK_INS_COUNT 5

struct instruction;

struct basic_block {
        struct ir_value val;

        struct vector ins_buf;
};

static inline 
struct basic_block *
ir_value_bb_get(struct ir_value *val)
{
        assert(val->type == ir_value_basic_block);
        return container_of(val, struct basic_block, val);
}

struct basic_block *ir_bb_create(const char *prefix, uint32_t index);

void ir_bb_destroy(struct basic_block *bb);

static inline _Bool ir_bb_empty(struct basic_block *bb)
{
        return vector_empty(&bb->ins_buf);
}

struct instruction *ir_bb_add_ins(struct basic_block *bb, 
                                  enum ir_ins_type type);

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
struct ir_value *
ir_bb_value(struct basic_block *block)
{
        return &block->val;
}

void ir_bb_ctf(struct basic_block *prev, struct basic_block *first, 
               struct basic_block *second, struct ir_object *condition);

static inline void ir_bb_ctf_uncond(struct basic_block *before, 
                                    struct basic_block *after)
{
        ir_bb_ctf(before, after, NULL, NULL);
}

#endif /* _IR_OBJECT_H_ */