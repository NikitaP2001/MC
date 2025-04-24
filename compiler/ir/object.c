#include <ir/basic_block.h>
#include <ir/value.h>
#include <ir/object.h>

struct ir_object *
ir_obj_create(const char *prefix, uint32_t index)
{
        struct ir_object *s_val = calloc(1, sizeof(struct ir_object));
        struct ir_value_params params = {
                .prefix = prefix,
                .index = index,
                .type = ir_value_object,
        };
        ir_value_init(&s_val->val, &params);
        s_val->eval = NULL;
        s_val->type = ir_object_unspecified;
        return s_val;
}

struct ir_value *ir_obj_value_get(struct ir_object *obj)
{
        return &obj->val;
}

static inline 
_Bool ir_obj_scalar_compat(struct ir_object *source, struct ir_object *result)
{
        struct ir_scalar src = source->var.scalar;
        struct ir_scalar res = result->var.scalar;
        /* currently we assume no type cast */
        return ir_scalar_type_compatible(src, res);
}

mc_status_t ir_obj_const_move(struct ir_object *source, struct ir_object *result)
{
        assert(ir_obj_const_eval(source) && ir_obj_const_eval(result));
        mc_status_t status = MC_FAIL;
        /* currently we assume no type cast */
        if (ir_obj_scalar_compat(source, result)) {
                result->var.scalar = source->var.scalar;
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_obj_move(struct basic_block *bb, struct ir_object *src, 
        struct ir_object *dest)
{
        mc_status_t status = MC_FAIL;
        if (ir_obj_scalar_compat(src, dest)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_mov);
                ir_ins_insert_use(ins, &dest->val);
                ir_ins_insert_use(ins, &src->val);
                status = MC_OK;
        }
        return status;
}

void ir_obj_sext(struct basic_block *bb, struct ir_object *src, 
                    struct ir_object *dest, enum scalar_type type)
{
        struct instruction *ins = NULL;
        assert(ir_obj_scalar_get(src).type < type);
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

void ir_obj_zext(struct basic_block *bb, struct ir_object *src, 
                    struct ir_object *dest, enum scalar_type type)
{
        struct instruction *ins = NULL;
        assert(ir_obj_scalar_get(src).type < type);
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

mc_status_t ir_obj_const_set(struct ir_object *dest, struct ir_scalar value)
{
        mc_status_t status = MC_FAIL;
        if (ir_scalar_type_compatible(dest->var.scalar, value)) {
                dest->var.scalar = value;
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_obj_or(struct basic_block *bb, struct ir_object *val1, 
                  struct ir_object *val2, struct ir_object *result)
{
        mc_status_t status = MC_FAIL;
        if (ir_obj_scalar_compat(val1, result) 
                && ir_obj_scalar_compat(val2, result)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_or);
                ir_ins_insert_use(ins, &result->val);
                ir_ins_insert_use(ins, &val1->val);
                ir_ins_insert_use(ins, &val2->val);
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_obj_xor(struct basic_block *bb, struct ir_object *val1, 
                          struct ir_object *val2, struct ir_object *result)
{
        mc_status_t status = MC_FAIL;
        if (ir_obj_scalar_compat(val1, result) 
                && ir_obj_scalar_compat(val2, result)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_xor);
                ir_ins_insert_use(ins, &result->val);
                ir_ins_insert_use(ins, &val1->val);
                ir_ins_insert_use(ins, &val2->val);
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_obj_and(struct basic_block *bb, struct ir_object *val1, 
                          struct ir_object *val2, struct ir_object *result)
{
        mc_status_t status = MC_FAIL;
        if (ir_obj_scalar_compat(val1, result) 
                && ir_obj_scalar_compat(val2, result)) {
                struct instruction *ins = ir_bb_add_ins(bb, ir_ins_and);
                ir_ins_insert_use(ins, &result->val);
                ir_ins_insert_use(ins, &val1->val);
                ir_ins_insert_use(ins, &val2->val);
                status = MC_OK;
        }
        return status;
}

mc_status_t ir_obj_cmp(struct basic_block *bb, enum ir_ins_type type, 
        struct ir_object *val1, struct ir_object *val2, struct ir_object *result)
{
        mc_status_t status = MC_FAIL;
        if (ir_obj_is_integer(result)) {
                struct instruction *ins = ir_bb_add_ins(bb, type);
                ir_ins_insert_use(ins, &result->val);
                ir_ins_insert_use(ins, &val1->val);
                ir_ins_insert_use(ins, &val2->val);
                status = MC_OK;
        }
        return status;
}