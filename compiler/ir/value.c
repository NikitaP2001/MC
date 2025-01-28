#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ir/value.h>
#include <ir/ins.h>

static void ir_bb_destroy(struct basic_block *bb);

void ir_value_add_use(struct value *val, struct ir_ins_use *use)
{
        if (val->use_count >= val->use_capacity) {
                val->use_capacity *= 2;
                val->uses = realloc(val->uses, val->use_capacity);
        }
        val->uses[val->use_count++] = use;
}

hash_key_t ir_value_hash(struct hlist_entry *node)
{
        return ir_value_hlist_entry(node)->hash;
}

void ir_value_free(struct hlist_entry *node)
{
        struct value *val = ir_value_hlist_entry(node);
        switch (val->type) {
                case ir_value_basic_block:
                        ir_bb_destroy(ir_value_bb_get(val));
                        break;
                default:
                        MC_DBG(MC_ERR, "unexpected value type");
        }
}

void ir_value_init(struct value *val, const char *prefix, uint32_t index)
{
        char *name = val->name;
        int len = snprintf(name, IR_VALUE_NAME_MAX, "%s%d", prefix, index);
        if (len < 0) {
                MC_DBG(MC_ERR, "form name failed");
                return;
        };
        val->uses = calloc(IR_VALUE_USE_COUNT, sizeof(struct ir_ins_use *));
        val->use_capacity = IR_VALUE_USE_COUNT;
        val->use_count = 0;
}

struct basic_block *ir_bb_create(const char *prefix, uint32_t index)
{
        struct basic_block *bb = calloc(1, sizeof(struct basic_block));
        ir_value_init(&bb->val, prefix, index);
        bb->val.type = ir_value_basic_block;
        vector_init(&bb->ins_buf, IR_BASIC_BLOCK_INS_COUNT, 
                sizeof(struct instruction));
        return bb;
}

static struct instruction *ir_bb_add_ins(struct basic_block *bb, 
        enum ir_ins_type type)
{
        struct instruction ins_init = {
                .type = type,
        };
        vector_push_back(&bb->ins_buf, &ins_init);
        return vector_last(&bb->ins_buf);
}

static void ir_bb_destroy(struct basic_block *bb)
{
        vector_free(&bb->ins_buf);
        free(bb);
}

void ir_bb_ctf(struct basic_block *prev, struct basic_block *first, 
               struct basic_block *second, struct lvalue *condition)
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

struct lvalue *
ir_lvalue_create(const char *prefix, uint32_t index)
{
        struct lvalue *s_val = calloc(1, sizeof(struct lvalue));
        ir_value_init(&s_val->val, prefix, index);
        s_val->eval = NULL;
        s_val->type = lvalue_unspecified;
        return s_val;
}

static inline 
_Bool ir_lvalue_scalar_compat(struct lvalue *source, struct lvalue *result)
{
        struct scalar_var src = source->var.scalar;
        struct scalar_var res = result->var.scalar;
        /* currently we assume no type cast */
        return ir_scalar_type_compatible(src, res);
}

mc_status_t ir_lvalue_const_move(struct lvalue *source, struct lvalue *result)
{
        assert(ir_lvalue_const_eval(source) && ir_lvalue_const_eval(result));
        mc_status_t status = MC_FAIL;
        /* currently we assume no type cast */
        if (ir_lvalue_scalar_compat(source, result)) {
                result->var.scalar = source->var.scalar;
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_lvalue_move(struct basic_block *bb, struct lvalue *src, 
        struct lvalue *dest)
{
        mc_status_t status = MC_FAIL;
        if (ir_lvalue_scalar_compat(src, dest)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_mov);
                ir_ins_insert_use(ins, &dest->val);
                ir_ins_insert_use(ins, &src->val);
                status = MC_OK;
        }
        return status;
}

void ir_lvalue_sext(struct basic_block *bb, struct lvalue *src, 
                    struct lvalue *dest, enum scalar_type type)
{
        struct instruction *ins = NULL;
        assert(ir_lvalue_scalar_get(src).type < type);
        switch (type) {
                case s_i16:
                        ins = ir_bb_add_ins(bb, ir_ins_sext16);
                        break;
                case s_i32:
                        ins = ir_bb_add_ins(bb, ir_ins_sext32);
                        break;
                case s_i64:
                        ins = ir_bb_add_ins(bb, ir_ins_sext64);
                        break;
                default:
                        MC_DBG(MC_CRIT, "unexpected type");
                        return;
        }
        ir_ins_insert_use(ins, &dest->val);
        ir_ins_insert_use(ins, &src->val);
}

void ir_lvalue_zext(struct basic_block *bb, struct lvalue *src, 
                    struct lvalue *dest, enum scalar_type type)
{
        struct instruction *ins = NULL;
        assert(ir_lvalue_scalar_get(src).type < type);
        switch (type) {
                case s_i16:
                        ins = ir_bb_add_ins(bb, ir_ins_zext16);
                        break;
                case s_i32:
                        ins = ir_bb_add_ins(bb, ir_ins_zext32);
                        break;
                case s_i64:
                        ins = ir_bb_add_ins(bb, ir_ins_zext64);
                        break;
                default:
                        MC_DBG(MC_CRIT, "unexpected type");
                        return;
        }
        ir_ins_insert_use(ins, &dest->val);
        ir_ins_insert_use(ins, &src->val);
}

mc_status_t ir_lvalue_const_set(struct lvalue *dest, struct scalar_var value)
{
        mc_status_t status = MC_FAIL;
        if (ir_scalar_type_compatible(dest->var.scalar, value)) {
                dest->var.scalar = value;
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_lvalue_or(struct basic_block *bb, struct lvalue *val1, 
                  struct lvalue *val2, struct lvalue *result)
{
        mc_status_t status = MC_FAIL;
        if (ir_lvalue_scalar_compat(val1, result) 
                && ir_lvalue_scalar_compat(val2, result)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_or);
                ir_ins_insert_use(ins, &result->val);
                ir_ins_insert_use(ins, &val1->val);
                ir_ins_insert_use(ins, &val2->val);
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_lvalue_xor(struct basic_block *bb, struct lvalue *val1, 
                          struct lvalue *val2, struct lvalue *result)
{
        mc_status_t status = MC_FAIL;
        if (ir_lvalue_scalar_compat(val1, result) 
                && ir_lvalue_scalar_compat(val2, result)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_xor);
                ir_ins_insert_use(ins, &result->val);
                ir_ins_insert_use(ins, &val1->val);
                ir_ins_insert_use(ins, &val2->val);
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_lvalue_and(struct basic_block *bb, struct lvalue *val1, 
                          struct lvalue *val2, struct lvalue *result)
{
        mc_status_t status = MC_FAIL;
        if (ir_lvalue_scalar_compat(val1, result) 
                && ir_lvalue_scalar_compat(val2, result)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_and);
                ir_ins_insert_use(ins, &result->val);
                ir_ins_insert_use(ins, &val1->val);
                ir_ins_insert_use(ins, &val2->val);
                status = MC_OK;
        }
        return status;
}