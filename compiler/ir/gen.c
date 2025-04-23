#include <parser/symtable.h>
#include <ir/gen.h>

/*
struct declaration *irgen_get_declaration(struct irgen_context *gen)
{
        return symtable_get_declaration(&gen->parser->id_tbl, id, scope);
}
*/

struct label *irgen_get_label(struct irgen_context *gen, struct pt_node *scope)
{
        struct token *id;
        struct pt_node *label;

        if (scope->sym == psym_labeled_statement)
                label = pt_node_child_first(scope);
        else if (scope->sym == psym_jump_statement && 
                 pt_node_child_first(scope)->sym == PARSER_KEYWORD(keyw_goto))
                label = pt_node_child_last(scope);
        else
                assert(false);
        id = label->node_value.value;
        return symtable_get_label(&gen->parser->sym_tbl, id, scope);
}

void irgen_init(struct irgen_context *gen, struct parser *ps)
{
        memset(gen, 0, sizeof(struct irgen_context));
        gen->mod = ir_module_create();
        gen->parser = ps;

        stack_init(&gen->switch_sets, IRGEN_STACK_CAPACITY, 
                   sizeof(struct switch_label_set*));
}

void irgen_free(struct irgen_context *gen)
{
        ir_module_destroy(gen->mod);
        struct stack *st = &gen->switch_sets;
        while (!stack_empty(st)) {
                struct switch_label_set *set;
                stack_top(st, &set);
                stack_pop(st);
                irgen_switch_label_set_free(set);
        }
        stack_free(&gen->switch_sets);
}

void irgen_function_create(struct irgen_context *gen, 
                           struct pt_node *func_def)
{
        assert(func_def->sym == psym_function_definition);
        struct function *func = ir_function_create(gen->mod, func_def);
        gen->curr_func = func;
}

/* Create new block based on current node, store it to current
 * analysed function and set it for all subsequent code to be 
 * inserted into it (or appended in case of new basic blocks)
 * source - ast node which will lead to this basic block
 *      @return: replaced basic block, which was previously on top */
struct basic_block *irgen_bb_create(struct irgen_context *gen, 
                                    struct pt_node *source)
{
        const char *preffix = NULL;
        _Bool is_first = false;
        switch (source->sym) {
                case psym_function_definition:
                        is_first = true;
                        preffix = "entry";
                        break;
                default:
                        preffix = "bb";
                        break;
        }

        struct basic_block *bb = ir_bb_create(preffix, irgen_label_index(gen));
        if (irgen_value_type(gen) == ir_value_basic_block) {
                struct basic_block *bb_prev = irgen_bb_get(gen);
                if (!is_first && !ir_bb_is_complete(bb_prev ))
                        ir_bb_ctf_uncond(bb_prev, bb);
        }
        
        gen->curr_value = &bb->val;
        ir_function_value_add(gen->curr_func, gen->curr_value);
        return bb;
}

static inline 
enum scalar_type 
irgen_const_type_to_scalar(enum constant_type type)
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
                        return scalar_invalid;
        }
}

#define IRGEN_INTEGER_EXPRESSION                \
        [psym_constant_expression] = 1,         \
        [psym_logical_or_expression] = 1,       \
        [psym_logical_and_expression] = 1,      \
        [psym_equality_expression] = 1,         \
        [psym_relational_expression] = 1,       \

static void irgen_obj_specify_type(struct ir_object *s_val, 
        struct pt_node *lval_node)
{
        enum parser_symbol sym = lval_node->sym;
        static const uint8_t int_expr[] = { IRGEN_INTEGER_EXPRESSION };

        if (int_expr[sym]) {
                struct ir_scalar scalar = {
                        .type = s_i32,
                        .is_signed = true,
                };
                ir_obj_scalar_set(s_val, scalar);
        } else if (sym == psym_constant) {
                struct token *const_tok = lval_node->node_value.value;
                enum constant_type type = token_constant(const_tok).type;
                struct ir_scalar scalar = {
                        .type = irgen_const_type_to_scalar(type),
                        .is_signed = token_const_is_signed(type)
                };
                ir_obj_scalar_set(s_val, scalar);
        } else if (sym == psym_type_name) {
                assert(false);
        }
        s_val->node = lval_node;
}

/* set new value type, based on associated @pt_node if it could
 * be unambiguously defined, leave unspecified othervise */
struct ir_object *irgen_obj_create(struct irgen_context *gen, 
        struct pt_node *lval_node)
{
        const char *prefix = "lvalue";
        struct ir_object *s_val = ir_obj_create(prefix, 
                irgen_label_index(gen));
        gen->curr_value = &s_val->val;
        if (lval_node != NULL)
                irgen_obj_specify_type(s_val, lval_node);
        ir_function_value_add(gen->curr_func, gen->curr_value);
        return s_val;
}

struct ir_object *irgen_scalar_create(struct irgen_context *gen, 
        struct ir_scalar var)
{
        struct ir_object *result = irgen_obj_create(gen, NULL);
        ir_obj_scalar_set(result, var);
        return result;
}


/* Return a single block in DAG, all path of @bb will lead to,
 * will insert uncontional jumps, if needed. 
 * Constrains: DAG */
struct basic_block *ir_bb_form_final(struct irgen_context *gen, 
        struct basic_block *bb)
{
        struct instruction *ctf_ins;
        struct ir_ins_use *op;
        struct stack bb_stack;
        if (!ir_bb_is_complete(bb))
                return bb;

        /* Try handle uncond branches first */
        while (ir_bb_is_complete(bb)) {
                ctf_ins = ir_bb_ins_last(bb);
                assert(ctf_ins->type == ir_ins_ctf);
                op = ctf_ins->use;
                /* we have branch since here, so we need stack */
                if (op->val->type == ir_value_basic_block)
                        break;
                bb = ir_value_bb_get(op->val);
        }

        if (!ir_bb_is_complete(bb))
                return bb;
       
        stack_init(&bb_stack, IRGEN_STACK_CAPACITY, 
                sizeof(struct basic_block *));

        struct basic_block *bb_final = irgen_bb_create(gen, NULL);
        /* note: if we went forever here, then probably graph had cycle */
        do {
                /* path ended, jump to final, and switch other path */
                while (!ir_bb_is_complete(bb)) {
                        ir_bb_ctf_uncond(bb, bb_final);
                        stack_top(&bb_stack, &bb);
                        stack_pop(&bb_stack);
                        if (stack_empty(&bb_stack))
                                return bb_final;
                }

                ctf_ins = ir_bb_ins_last(bb);
                assert(ctf_ins->type == ir_ins_ctf);
                op = ctf_ins->use;

                /* met uncond transfer here */
                if (op->val->type == ir_value_basic_block) {
                        bb = ir_value_bb_get(op->val);
                        continue;
                }

                op = list_next(op);
                bb = ir_value_bb_get(op->val);
                stack_push(&bb_stack, &bb);
                op = list_next(op);
                bb = ir_value_bb_get(op->val);
        } while (!stack_empty(&bb_stack));
        stack_free(&bb_stack);
        return bb_final;
}

/* Specify if @result has const evaluation, or set it`s basic block.
 * In result of some generation procedure, we will have @result, 
 * if evaluation could be const, or block of code which leads to 
 * forming @result value otherwise */
static inline void irgen_obj_set_eval(struct irgen_context *gen, 
                                         struct ir_object *result)
{
        /* we have non-const evaluation */
        if (irgen_value_type(gen) == ir_value_basic_block)
                ir_obj_set_eval(result, irgen_bb_get(gen));
        else
                ir_obj_set_eval(result, NULL);
}

mc_status_t irgen_obj_move_single(struct irgen_context *gen, 
                                     struct ir_object *val_mov)
{
        mc_status_t status = MC_OK;

        struct ir_object *result = irgen_obj_get(gen);
        struct basic_block *bb_val = ir_obj_get_eval(val_mov);
        if (bb_val == NULL) {
                status = ir_obj_const_move(val_mov, result);
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_mov = ir_bb_form_final(gen, bb_val);
                status = ir_obj_move(bb_mov, val_mov, result);
                if (!MC_SUCC(status))
                        goto fail;

                irgen_value_set(gen, &bb_val->val);
                irgen_obj_set_eval(gen, result);
        }
        return status;
fail:
        return IRGEN_ERROR(gen, val_mov->node, "incompatible types");
}

mc_status_t irgen_obj_move_cond(struct irgen_context *gen, 
                                   struct ir_object *condition, 
                                   struct ir_object *value_true, 
                                   struct ir_object *value_false)
{
        mc_status_t status = MC_OK;
        struct ir_object *result = irgen_obj_get(gen);

        struct basic_block *bb_val_true = ir_obj_get_eval(value_true);
        struct basic_block *bb_val_false = ir_obj_get_eval(value_false);
        if (ir_obj_const_eval(condition)) {
                if (ir_scalar_eval_true(ir_obj_scalar_get(condition)))
                        status = irgen_obj_move_single(gen, value_true);
                else
                        status = irgen_obj_move_single(gen, value_false);
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_mov_true;
                struct basic_block *bb_mov_false;
                /* true value is const eval */
                if (bb_val_true == NULL) {
                        bb_mov_true = irgen_bb_create(gen, NULL);
                        bb_val_true = bb_mov_true;
                } else {
                        bb_mov_true = ir_bb_form_final(gen, bb_val_true);
                }
                /* false value is const eval */
                if (bb_val_false == NULL) {
                        bb_mov_false = irgen_bb_create(gen, NULL);
                        bb_val_false = bb_mov_false;
                } else {
                        bb_mov_false = ir_bb_form_final(gen, bb_val_false);
                }

                ir_bb_ctf(bb_jmp, bb_val_true, bb_val_false, condition);

                status = ir_obj_move(bb_mov_true, value_true, result);
                if (MC_SUCC(status)) {
                        return IRGEN_ERROR(gen, value_true->node, 
                                "incompatible types");
                }

                status = ir_obj_move(bb_mov_false, value_false, result);
                if (MC_SUCC(status)) {
                        return IRGEN_ERROR(gen, value_false->node, 
                                "incompatible types");
                }

                irgen_value_set(gen, &bb_jmp->val);
                irgen_obj_set_eval(gen, result);
        }
        return status;
}

/* result is applied to @val */
mc_status_t irgen_scalar_cast(struct irgen_context *gen,
                              struct ir_object *val, 
                              enum scalar_type type)
{
        mc_status_t status = MC_OK;
        struct basic_block *bb_val = ir_obj_get_eval(val);
        struct ir_scalar scalar = ir_obj_scalar_get(val);
        /* TODO: implement for narrowing types */
        assert(scalar.type < type);
        /* TODO: implement for float types */
        assert(ir_scalar_is_integer(scalar));
        if (bb_val == NULL) {
                status = ir_obj_scalar_const_cast(val, type);
                if (!MC_SUCC(status))
                        goto fail;
        } else if (scalar.type != type) {
                struct basic_block *bb_cast = ir_bb_form_final(gen, bb_val);
                if (scalar.is_signed)
                        ir_obj_sext(bb_cast, val, val, type);
                else
                        ir_obj_zext(bb_cast, val, val, type);
        }
        return status;
fail:
        return IRGEN_ERROR(gen, val->node, 
                "types are incompatible for cast operation");
}

static void irgen_scalar_integer_promotion(struct irgen_context *gen,
                                           struct ir_object *val1, 
                                           struct ir_object *val2)
{
        struct ir_scalar val1_scalar = ir_obj_scalar_get(val1);
        struct ir_scalar val2_scalar = ir_obj_scalar_get(val2);
        enum scalar_type type1 = val1_scalar.type;
        enum scalar_type type2 = val2_scalar.type;
        if (type1 == type2)
                return;
        _Bool val1_signed = val1_scalar.is_signed;
        _Bool val2_signed = val2_scalar.is_signed;
        if (val1_signed == val2_signed) {
                if (type1 < type2)
                        irgen_scalar_cast(gen, val1, type2);
                else
                        irgen_scalar_cast(gen, val2, type1);
        } else if (!val1_signed && type1 >= type2) {
                val2->var.scalar.is_signed = false;
                irgen_scalar_cast(gen, val2, type1);
        } else if (!val2_signed && type2 >= type1) {
                val1->var.scalar.is_signed = false;
                irgen_scalar_cast(gen, val1, type2);
        } else if (type1 != type2) {
                if (type1 < type2) {
                        val1->var.scalar.is_signed = true;
                        irgen_scalar_cast(gen, val1, type2);
                } else {
                        val2->var.scalar.is_signed = true;
                        irgen_scalar_cast(gen, val2, type1);
                }
        } else {
                MC_DBG(MC_CRIT, "unexpected case");
        }

}

mc_status_t irgen_obj_and(struct irgen_context *gen, 
                             struct ir_object *val1, 
                             struct ir_object *val2)
{
        struct ir_object *result = irgen_obj_get(gen);
        mc_status_t status;

        /* The usual arithmetic conversions are performed on the operands */
        irgen_scalar_integer_promotion(gen, val1, val2);

        if (!ir_scalar_type_compatible(ir_obj_scalar_get(result),
                ir_obj_scalar_get(val1))) {
                return IRGEN_ERROR(gen, result->node,
                                "incompatible result type");
        }

        if (ir_obj_const_eval(val1) && ir_obj_const_eval(val2)) {
                status = ir_scalar_and(ir_obj_scalar_get(val1),
                        ir_obj_scalar_get(val2),
                        &result->var.scalar);
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_jmp_start = bb_jmp;
                struct basic_block *val_bb;

                if (!ir_obj_const_eval(val1)) {
                        val_bb = ir_obj_get_eval(val1);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }
                if (!ir_obj_const_eval(val2)) {
                        val_bb = ir_obj_get_eval(val2);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }

                status = ir_obj_and(bb_jmp, val1, val2, result);
                if (!MC_SUCC(status))
                        goto fail;
                irgen_value_set(gen, ir_bb_value(bb_jmp_start));
                irgen_obj_set_eval(gen, result);
        }

        return MC_OK;
fail:
        return IRGEN_ERROR(gen, result->node,
                "invalid bitwise and operation");
}

mc_status_t irgen_obj_xor(struct irgen_context *gen, 
                             struct ir_object *val1, 
                             struct ir_object *val2)
{
        struct ir_object *result = irgen_obj_get(gen);
        mc_status_t status;

        /* The usual arithmetic conversions are performed on the operands */
        irgen_scalar_integer_promotion(gen, val1, val2);

        if (!ir_scalar_type_compatible(ir_obj_scalar_get(result),
                ir_obj_scalar_get(val1))) {
                return IRGEN_ERROR(gen, result->node,
                                "incompatible result type");
        }

        if (ir_obj_const_eval(val1) && ir_obj_const_eval(val2)) {
                status = ir_scalar_xor(ir_obj_scalar_get(val1),
                        ir_obj_scalar_get(val2),
                        &result->var.scalar);
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_jmp_start = bb_jmp;
                struct basic_block *val_bb;

                if (!ir_obj_const_eval(val1)) {
                        val_bb = ir_obj_get_eval(val1);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }
                if (!ir_obj_const_eval(val2)) {
                        val_bb = ir_obj_get_eval(val2);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }

                status = ir_obj_xor(bb_jmp, val1, val2, result);
                if (!MC_SUCC(status))
                        goto fail;
                irgen_value_set(gen, ir_bb_value(bb_jmp_start));
                irgen_obj_set_eval(gen, result);
        }

        return MC_OK;
fail:
        return IRGEN_ERROR(gen, result->node,
                "invalid bitwise xor operation");
}

mc_status_t irgen_obj_or(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2)
{
        struct ir_object *result = irgen_obj_get(gen);
        mc_status_t status;
        /* The usual arithmetic conversions are performed on the operands */
        irgen_scalar_integer_promotion(gen, val1, val2);

        if (!ir_scalar_type_compatible(ir_obj_scalar_get(result), 
                ir_obj_scalar_get(val1))) {
                return IRGEN_ERROR(gen, result->node, 
                                "incompatible result type");
        }

        if (ir_obj_const_eval(val1) && ir_obj_const_eval(val2)) {
                status = ir_scalar_or(ir_obj_scalar_get(val1), 
                        ir_obj_scalar_get(val2), 
                        &result->var.scalar);
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_jmp_start = bb_jmp;
                struct basic_block *val_bb;

                if (!ir_obj_const_eval(val1)) {
                        val_bb = ir_obj_get_eval(val1);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }
                if (!ir_obj_const_eval(val2)) {
                        val_bb = ir_obj_get_eval(val2);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }

                status = ir_obj_or(bb_jmp, val1, val2, result);
                if (!MC_SUCC(status))
                        goto fail;
                irgen_value_set(gen, ir_bb_value(bb_jmp_start));
                irgen_obj_set_eval(gen, result);
        }

        return MC_OK;
fail:
        return IRGEN_ERROR(gen, result->node, 
                "invalid bitwise or operation");
}

mc_status_t irgen_obj_log_and(struct irgen_context *gen, 
                                struct ir_object *val1, 
                                struct ir_object *val2)
{
        mc_status_t status = MC_OK;

        struct ir_object *result = irgen_obj_get(gen);
        struct basic_block *bb_val1 = ir_obj_get_eval(val1);
        struct basic_block *bb_val2 = ir_obj_get_eval(val2);
        _Bool val1_false = (bb_val1 == NULL && !ir_obj_eval_true(val1));
        _Bool val2_false = (bb_val2 == NULL && !ir_obj_eval_true(val2));

        if (val1_false || (bb_val1 == NULL && val2_false)) {
                status = ir_obj_const_set(result, ir_scalar_create_int(0));
                if (!MC_SUCC(status))
                        goto fail;
        } else if (bb_val1 == NULL && !val1_false 
                && bb_val2 == NULL && !val2_false) {
                status = ir_obj_const_set(result, ir_scalar_create_int(1));
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_jmp_start = bb_jmp;
                struct basic_block *bb_val_false = irgen_bb_create(gen, NULL);
                struct basic_block *bb_true = NULL;

                struct ir_object *val_0 = irgen_scalar_create(gen, 
                        ir_scalar_create_int(0));
                status = ir_obj_move(bb_val_false, val_0, result);
                if (!MC_SUCC(status))
                        goto fail;

                if (!val2_false) {
                        bb_true = irgen_bb_create(gen, NULL);
                        struct ir_object *val_1 = irgen_scalar_create(gen, 
                                ir_scalar_create_int(1));
                        status = ir_obj_move(bb_true, val_1, result);
                        if (!MC_SUCC(status))
                                goto fail;
                }

                if (bb_val1 != NULL) {
                        /* generate code to calculate val1 */
                        ir_bb_ctf_uncond(bb_jmp, bb_val1);
                        struct basic_block *bb_val1_after 
                                = ir_bb_form_final(gen, bb_val1);
                        if (bb_true == NULL) {
                                ir_bb_ctf_uncond(bb_val1_after, bb_val_false);
                        } else {
                                ir_bb_ctf(bb_val1_after, bb_true, 
                                        bb_val_false, val1);
                        }
                                
                        if (bb_val2 != NULL) {
                                bb_jmp = bb_true;
                                bb_true = irgen_bb_create(gen, NULL);
                        }
                }

                if (bb_val2 != NULL) {
                        ir_bb_ctf_uncond(bb_jmp, bb_val2);
                        struct basic_block *bb_val2_after 
                                = ir_bb_form_final(gen, bb_val2);
                        ir_bb_ctf(bb_val2_after, bb_true, bb_val_false, val2);
                }

                irgen_value_set(gen, ir_bb_value(bb_jmp_start));
                irgen_obj_set_eval(gen, result);
        }

        return status;
fail:
        return IRGEN_ERROR(gen, val1->node, "incompatible types");
}

mc_status_t irgen_obj_log_or(struct irgen_context *gen, 
                                struct ir_object *val1, 
                                struct ir_object *val2)
{
        mc_status_t status = MC_OK;

        struct ir_object *result = irgen_obj_get(gen);
        struct basic_block *bb_val1 = ir_obj_get_eval(val1);
        struct basic_block *bb_val2 = ir_obj_get_eval(val2);
        _Bool val1_true = (bb_val1 == NULL && ir_obj_eval_true(val1));
        _Bool val2_true = (bb_val2 == NULL && ir_obj_eval_true(val2));

        if (val1_true || (bb_val1 == NULL && val2_true)) {
                status = ir_obj_const_set(result, ir_scalar_create_int(1));
                if (!MC_SUCC(status))
                        goto fail;
        } else if (bb_val1 == NULL && !val1_true 
                && bb_val2 == NULL && !val2_true) {
                status = ir_obj_const_set(result, ir_scalar_create_int(0));
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_jmp_start = bb_jmp;
                struct basic_block *bb_val_true = irgen_bb_create(gen, NULL);
                struct basic_block *bb_false = NULL;

                struct ir_object *val_1 = irgen_scalar_create(gen, 
                        ir_scalar_create_int(1));
                status = ir_obj_move(bb_val_true, val_1, result);
                if (!MC_SUCC(status))
                        goto fail;

                if (!val2_true) {
                        bb_false = irgen_bb_create(gen, NULL);
                        struct ir_object *val_0 = irgen_scalar_create(gen, 
                                ir_scalar_create_int(0));
                        status = ir_obj_move(bb_false, val_0, result);
                        if (!MC_SUCC(status))
                                goto fail;
                }

                if (bb_val1 != NULL) {
                        /* generate code to calculate val1 */
                        ir_bb_ctf_uncond(bb_jmp, bb_val1);
                        struct basic_block *bb_val1_after 
                                = ir_bb_form_final(gen, bb_val1);
                        if (bb_false == NULL)
                                ir_bb_ctf_uncond(bb_val1_after, bb_val_true);
                        else
                                ir_bb_ctf(bb_val1_after, bb_val_true, 
                                        bb_false, val1);
                        if (bb_val2 != NULL) {
                                bb_jmp = bb_false;
                                bb_false = irgen_bb_create(gen, NULL);
                        }
                }

                if (bb_val2 != NULL) {
                        ir_bb_ctf_uncond(bb_jmp, bb_val2);
                        struct basic_block *bb_val2_after 
                                = ir_bb_form_final(gen, bb_val2);
                        ir_bb_ctf(bb_val2_after, bb_val_true, bb_false, val2);
                }

                irgen_value_set(gen, ir_bb_value(bb_jmp_start));
                irgen_obj_set_eval(gen, result);
        }

        return status;
fail:
        return IRGEN_ERROR(gen, val1->node, "incompatible types");
}

static mc_status_t irgen_obj_cmp(struct irgen_context *gen, 
                                    struct ir_object *val1, 
                                    struct ir_object *val2,
                                    enum ir_ins_type cmp_type)
{
        mc_status_t status = MC_OK;
        struct ir_object *result = irgen_obj_get(gen);
        assert(ir_obj_is_integer(result));

        /* Complex number comparisons are not supported now */
        assert(ir_obj_scalar_get(val1).type != s_complex
                && ir_obj_scalar_get(val2).type != s_complex);

        if (ir_obj_is_arithmetic(val1)) {
                assert(ir_obj_is_arithmetic(val2));
                irgen_scalar_integer_promotion(gen, val1, val2);
        }

        if (ir_obj_const_eval(val1) && ir_obj_const_eval(val2)) {
                struct ir_scalar s_cmp_val;
                if (ir_scalar_const_cmp(ir_obj_scalar_get(val1), 
                        ir_obj_scalar_get(val2)))
                        s_cmp_val = ir_scalar_create_int(1);
                else
                        s_cmp_val = ir_scalar_create_int(0);
                ir_obj_const_set(result, s_cmp_val);
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_jmp_start = bb_jmp;
                struct basic_block *val_bb;

                if (!ir_obj_const_eval(val1)) {
                        val_bb = ir_obj_get_eval(val1);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }
                if (!ir_obj_const_eval(val2)) {
                        val_bb = ir_obj_get_eval(val2);
                        val_bb = ir_bb_form_final(gen, val_bb);
                        ir_bb_ctf_uncond(bb_jmp, val_bb);
                        bb_jmp = val_bb;
                }

                status = ir_obj_cmp(bb_jmp, cmp_type, val1, val2, result);
                if (!MC_SUCC(status))
                        goto fail;
                irgen_value_set(gen, ir_bb_value(bb_jmp_start));
                irgen_obj_set_eval(gen, result);
        }
        return status;
fail:
        return IRGEN_ERROR(gen, val1->node, "incompatible types");
}

mc_status_t irgen_obj_neq(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2)
{
        return irgen_obj_cmp(gen, val1, val2, ir_ins_cmp_ne);
}

mc_status_t irgen_obj_eq(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2)
{
        return irgen_obj_cmp(gen, val1, val2, ir_ins_cmp_eq);
}

mc_status_t irgen_obj_lt(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2)
{
        return irgen_obj_cmp(gen, val1, val2, ir_ins_cmp_lt); 
}

mc_status_t irgen_obj_gt(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2)
{
        return irgen_obj_cmp(gen, val1, val2, ir_ins_cmp_gt);
}

mc_status_t irgen_obj_le(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2)
{
        return irgen_obj_cmp(gen, val1, val2, ir_ins_cmp_le);
}

mc_status_t irgen_obj_ge(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2)
{
        return irgen_obj_cmp(gen, val1, val2, ir_ins_cmp_ge);
}

mc_status_t irgen_obj_lshift(struct irgen_context *gen,
                                struct ir_object *val1,
                                struct ir_object *val2)
{
        UNUSED(gen);
        UNUSED(val1);
        UNUSED(val2);
        return MC_OK;
}

mc_status_t irgen_obj_rshift(struct irgen_context *gen,
                                struct ir_object *val1,
                                struct ir_object *val2)
{
        UNUSED(gen);
        UNUSED(val1);
        UNUSED(val2);
        return MC_OK;
}

mc_status_t irgen_obj_add(struct irgen_context *gen,
                             struct ir_object *val1,
                             struct ir_object *val2)
{
        UNUSED(gen);
        UNUSED(val1);
        UNUSED(val2);
        return MC_OK;
}

mc_status_t irgen_obj_sub(struct irgen_context *gen,
                             struct ir_object *val1,
                             struct ir_object *val2)
{
        UNUSED(gen);
        UNUSED(val1);
        UNUSED(val2);
        return MC_OK;
}

mc_status_t irgen_obj_inc(struct irgen_context *gen,
                             struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_dec(struct irgen_context *gen,
                             struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_mul(struct irgen_context *gen,
                             struct ir_object *val1,
                             struct ir_object *val2)
{
        UNUSED(gen);
        UNUSED(val1);
        UNUSED(val2);
        return MC_OK;
}

mc_status_t irgen_obj_div(struct irgen_context *gen,
                             struct ir_object *val1,
                             struct ir_object *val2)
{
        UNUSED(gen);
        UNUSED(val1);
        UNUSED(val2);
        return MC_OK;
}

mc_status_t irgen_obj_mod(struct irgen_context *gen,
                             struct ir_object *val1,
                             struct ir_object *val2)
{
        UNUSED(gen);
        UNUSED(val1);
        UNUSED(val2);
        return MC_OK;
}

mc_status_t irgen_obj_type_cast(struct irgen_context *gen,
                                   struct ir_object *val,
                                   struct pt_node *type)
{
        UNUSED(gen);
        UNUSED(val);
        UNUSED(type);
        return MC_OK;
}

mc_status_t irgen_obj_sizeof(struct irgen_context *gen,
                                struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_sizeof_tname(struct irgen_context *gen,
                                      struct pt_node *tname)
{
        UNUSED(gen);
        UNUSED(tname);
        return MC_OK;
}

mc_status_t irgen_obj_addr_of(struct irgen_context *gen,
                                 struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_deref(struct irgen_context *gen,
                               struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_neg(struct irgen_context *gen,
                             struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_not(struct irgen_context *gen,
                             struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_log_not(struct irgen_context *gen,
                                 struct ir_object *val)
{
        UNUSED(gen);
        UNUSED(val);
        return MC_OK;
}

mc_status_t irgen_obj_struct_member_op(struct irgen_context *gen,
                                          struct ir_object *object,
                                          struct pt_node *member)
{
        UNUSED(gen);
        UNUSED(object);
        UNUSED(member);
        return MC_OK;
}

mc_status_t irgen_obj_struct_ptr_op(struct irgen_context *gen,
                                       struct ir_object *ptr_object,
                                       struct pt_node *member)
{
        UNUSED(gen);
        UNUSED(ptr_object);
        UNUSED(member);
        return MC_OK;
}

mc_status_t irgen_obj_array_index(struct irgen_context *gen,
                                     struct ir_object *array,
                                     struct ir_object *index)
{
        UNUSED(gen);
        UNUSED(array);
        UNUSED(index);
        return MC_OK;
}