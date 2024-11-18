#include <ir/gen.h>

struct declaration *irgen_get_declaration(struct irgen_context *gen)
{
        UNUSED(gen);
        return NULL;
}

struct label *irgen_get_label(struct irgen_context *gen, struct pt_node *scope)
{
        struct token *id;
        struct pt_node *label;

        if (scope->sym == psym_labeled_statement)
                label = pt_node_child_first(scope);
        else if (scope->sym == psym_jump_statement && pt_node_child_first(
                scope)->sym == PARSER_KEYWORD(keyw_goto))
                label = pt_node_child_last(scope);
        else
                assert(false);
        id = label->node_value.value;
        return symtable_get_label(&gen->parser->id_tbl, id, scope);
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

static void irgen_lvalue_specify_type(struct lvalue *s_val, 
        struct pt_node *lval_node)
{
        struct scalar_var scalar;
        enum parser_symbol sym = lval_node->sym;
        if (sym == psym_constant_expression
                || sym == psym_logical_or_expression
                || sym == psym_logical_and_expression) {
                s_val->type = lvalue_scalar;
                scalar.type = scalar_int;
                s_val->var.scalar = scalar;
        }
        s_val->node = lval_node;
}

struct lvalue *irgen_lvalue_create(struct irgen_context *gen, 
        struct pt_node *lval_node)
{
        const char *prefix = "lvalue";
        struct lvalue *s_val = ir_lvalue_create(prefix, 
                irgen_label_index(gen));
        gen->curr_value = &s_val->val;
        irgen_lvalue_specify_type(s_val, lval_node);
        ir_function_value_add(gen->curr_func, gen->curr_value);
        return s_val;
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

        /* try handle uncond branches first */
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
static inline void irgen_lvalue_set_eval(struct irgen_context *gen, 
                                         struct lvalue *result)
{
        /* we have non-const evaluation */
        if (irgen_value_type(gen) == ir_value_basic_block)
                ir_lvalue_set_eval(result, irgen_bb_get(gen));
        else
                ir_lvalue_set_eval(result, NULL);
}

mc_status_t irgen_lvalue_move_single(struct irgen_context *gen, 
                                     struct lvalue *val_mov)
{
        mc_status_t status = MC_OK;

        struct lvalue *result = irgen_lvalue_get(gen);
        struct basic_block *bb_val = ir_lvalue_get_eval(val_mov);
        if (bb_val == NULL) {
                status = ir_lvalue_const_move(val_mov, result);
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_mov = ir_bb_form_final(gen, bb_val);
                status = ir_lvalue_move(bb_mov, val_mov, result);
                if (!MC_SUCC(status))
                        goto fail;

                irgen_value_set(gen, &bb_val->val);
                irgen_lvalue_set_eval(gen, result);
        }
        return status;
fail:
        return IRGEN_ERROR(gen, val_mov->node, "incompatible types");
}

mc_status_t irgen_lvalue_move_cond(struct irgen_context *gen, 
                                   struct lvalue *condition, 
                                   struct lvalue *value_true, 
                                   struct lvalue *value_false)
{
        mc_status_t status = MC_OK;
        struct lvalue *result = irgen_lvalue_get(gen);

        struct basic_block *bb_val_true = ir_lvalue_get_eval(value_true);
        struct basic_block *bb_val_false = ir_lvalue_get_eval(value_false);
        if (ir_lvalue_const_eval(condition)) {
                if (ir_scalar_eval_true(condition))
                        status = irgen_lvalue_move_single(gen, value_true);
                else
                        status = irgen_lvalue_move_single(gen, value_false);
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

                status = ir_lvalue_move(bb_mov_true, value_true, result);
                if (MC_SUCC(status)) {
                        return IRGEN_ERROR(gen, value_true->node, 
                                "incompatible types");
                }

                status = ir_lvalue_move(bb_mov_false, value_false, result);
                if (MC_SUCC(status)) {
                        return IRGEN_ERROR(gen, value_false->node, 
                                "incompatible types");
                }

                irgen_value_set(gen, &bb_jmp->val);
                irgen_lvalue_set_eval(gen, result);
        }
        return status;
}

mc_status_t irgen_lvalue_log_or(struct irgen_context *gen, 
                                struct lvalue *val1, 
                                struct lvalue *val2)
{
        mc_status_t status = MC_OK;

        struct lvalue *result = irgen_lvalue_get(gen);
        struct basic_block *bb_val1 = ir_lvalue_get_eval(val1);
        struct basic_block *bb_val2 = ir_lvalue_get_eval(val2);
        _Bool val1_true = (bb_val1 == NULL && ir_scalar_eval_true(val1));
        _Bool val2_true = (bb_val2 == NULL && ir_scalar_eval_true(val2));

        struct lvalue *val_1 = irgen_scalar_create(gen, 
                        scalar_create_int(1));
        if (val1_true || val2_true) {
                status = ir_lvalue_const_move(val_1, result);
                if (!MC_SUCC(status))
                        goto fail;
        } else {
                struct basic_block *bb_jmp = irgen_bb_create(gen, NULL);
                struct basic_block *bb_jmp_start = bb_jmp;
                struct basic_block *bb_val_true = irgen_bb_create(gen, NULL);
                struct basic_block *bb_false = irgen_bb_create(gen, NULL);

                if (bb_val1 != NULL) {
                        /* generate code to calculate val1 */
                        ir_bb_ctf_uncond(bb_jmp, bb_val1);
                        struct basic_block *bb_val1_after 
                                = ir_bb_form_final(gen, bb_val1);
                        ir_bb_ctf(bb_val1_after, bb_val_true, bb_false, val1);
                        if (bb_val2 != NULL) {
                                bb_jmp = bb_false;
                                bb_false = irgen_bb_create(gen, NULL);
                        }
                }

                if (bb_val2 != NULL) {
                        ir_bb_ctf_uncond(bb_jmp, bb_val2);
                        struct basic_block *bb_val2_after 
                                = ir_bb_form_final(gen, bb_val1);
                        ir_bb_ctf(bb_val2_after, bb_val_true, bb_false, val2);
                }

                status = ir_lvalue_move(bb_val_true, val_1, result);
                if (!MC_SUCC(status))
                        goto fail;

                struct lvalue *val_0 = irgen_scalar_create(gen, 
                        scalar_create_int(0));

                status = ir_lvalue_move(bb_false, val_0, result);
                if (!MC_SUCC(status))
                        goto fail;

                irgen_value_set(gen, ir_bb_value(bb_jmp_start));
                irgen_lvalue_set_eval(gen, result);
        }

        return status;
fail:
        return IRGEN_ERROR(gen, val1->node, "incompatible types");
}