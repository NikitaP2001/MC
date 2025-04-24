#include <ir/basic_block.h>

struct basic_block *ir_bb_create(const char *prefix, uint32_t index)
{
        struct basic_block *bb = calloc(1, sizeof(struct basic_block));
        struct ir_value_params params = {
                .prefix = prefix,
                .index = index,
                .type = ir_value_basic_block,
        };
        ir_value_init(&bb->val, &params);
        bb->val.type = ir_value_basic_block;
        vector_init(&bb->ins_buf, IR_BASIC_BLOCK_INS_COUNT, 
                sizeof(struct instruction));
        return bb;
}

void ir_bb_destroy(struct basic_block *bb)
{
        vector_free(&bb->ins_buf);
        free(bb);
}

struct instruction *ir_bb_add_ins(struct basic_block *bb, 
                                  enum ir_ins_type type)
{
        struct instruction ins_init = {
                .type = type,
        };
        vector_push_back(&bb->ins_buf, &ins_init);
        return vector_last(&bb->ins_buf);
}

void ir_bb_ctf(struct basic_block *prev, struct basic_block *first, 
               struct basic_block *second, struct ir_object *condition)
{
        struct instruction *ins = ir_bb_add_ins(prev, ir_ins_ctf);
        if (condition == NULL) {
                ir_ins_insert_use(ins, &first->val);
        } else {
                ir_ins_insert_use(ins, &second->val);
                ir_ins_insert_use(ins, &first->val);
                ir_ins_insert_use(ins, &condition->val);
        } 
}

_Bool ir_bb_is_complete(struct basic_block *block)
{
        _Bool result = false;
        if (!ir_bb_empty(block)) {
                struct instruction *last_ins 
                        = vector_last(&block->ins_buf);
                result = ir_ins_is_control_transfer(last_ins);
        }
        return result; 
}

size_t ir_bb_ins_count(struct basic_block *block)
{
        return vector_size(&block->ins_buf);
}

struct instruction *ir_bb_ins_get(struct basic_block *block, size_t ins_num)
{
        return vector_get(&block->ins_buf, ins_num);
}