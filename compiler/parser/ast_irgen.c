#include <assert.h>
#include <ir/gen.h>
#include <ir.h>
#include <parser/ast.h>
#include <parser/symtable.h>

static mc_status_t irgen_compound_statement(struct irgen_context *gen, 
                                            struct pt_node *comp_stmt);

static mc_status_t irgen_statement(struct irgen_context *gen, 
                                   struct pt_node *stmt);

static mc_status_t irgen_cast_expression(struct irgen_context *gen, 
                                         struct pt_node *stmt);

static mc_status_t irgen_expression(struct irgen_context *gen, 
                                    struct pt_node *stmt)
{
        UNUSED(gen);
        UNUSED(stmt);
        return MC_FAIL;
}

static mc_status_t irgen_primary_expression(struct irgen_context *gen,
                                            struct pt_node *stmt)
{
        mc_status_t status = MC_FAIL;
        struct ir_object *result = NULL;
        struct pt_node *curr_node = pt_node_child_first(stmt);
        if (pt_node_sym_cmp(curr_node, psym_identifier)) {

                /*
                struct *value  = irgen
                */
        }
        irgen_value_set(gen, &result->val);
        return status;
}

static mc_status_t irgen_initializer_list(struct irgen_context *gen,
                                          struct pt_node *stmt)
{
        UNUSED(gen);
        UNUSED(stmt);
        return MC_FAIL;
}

static mc_status_t irgen_argument_expression_list(struct irgen_context *gen,
                                                  struct ir_object *func,
                                                  struct pt_node *arg_list)
{
        /* perform the function call */
        UNUSED(gen);
        UNUSED(func);
        UNUSED(arg_list);
        return MC_FAIL;
}

static mc_status_t irgen_postfix_expression(struct irgen_context *gen, 
                                            struct pt_node *stmt)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(stmt);
        uint16_t i_node = 1;
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *curr_node = pt_node_child_first(stmt);
        struct pt_node *id;
        struct ir_object *curr_val;
        struct ir_object *old_val;

        if (pt_node_sym_cmp(curr_node, psym_primary_expression)) {
                status = irgen_primary_expression(gen, curr_node);
                curr_val = irgen_obj_create(gen, curr_node);
        } else { 
                curr_val = irgen_obj_create(gen, curr_node);
                assert(pt_node_sym_cmp(curr_node, psym_type_name));
                curr_node = pt_node_child_number(stmt, ++i_node);
                status = irgen_initializer_list(gen, curr_node);
        }
        if (!MC_SUCC(status))
                return status;

        for (++i_node; i_node <= child_count; i_node++) {
                curr_node = pt_node_child_number(stmt, i_node);
                if (pt_node_sym_cmp(curr_node, psym_expression)) {
                        struct ir_object *index = irgen_obj_create(gen, curr_node);
                        status = irgen_expression(gen, curr_node);
                        if (!MC_SUCC(status))
                                return status;
                        irgen_value_set(gen, &curr_val->val); 
                        status = irgen_obj_array_index(gen, curr_val, index);
                } else if (pt_node_sym_cmp(curr_node, psym_argument_expression_list)) {
                        status = irgen_argument_expression_list(gen, 
                                curr_val, curr_node);
                } else if (pt_node_sym_cmp(curr_node, 
                        PARSER_PUNCTUATOR(punc_right_arrow))) {
                        assert(i_node + 1 <= child_count);
                        id = pt_node_child_number(stmt, i_node + 1);
                        status = irgen_obj_struct_ptr_op(gen, curr_val, id);
                } else if (pt_node_sym_cmp(curr_node, 
                        PARSER_PUNCTUATOR(punc_dot))) {
                        assert(i_node + 1 <= child_count);
                        id = pt_node_child_number(stmt, i_node + 1);
                        status = irgen_obj_struct_member_op(gen, curr_val, id);
                } else if (pt_node_sym_cmp(curr_node, 
                        PARSER_PUNCTUATOR(punc_increment))) {
                        old_val = curr_val;
                        curr_val = irgen_obj_create(gen, stmt);
                        status = irgen_obj_inc(gen, old_val);
                } else if (pt_node_sym_cmp(curr_node, 
                        PARSER_PUNCTUATOR(punc_decrement))) {
                        old_val = curr_val;
                        curr_val = irgen_obj_create(gen, stmt);
                        status = irgen_obj_dec(gen, old_val);
                } else {
                        MC_DBG(MC_CRIT, "unexpected postfix operator");
                }
                if (!MC_SUCC(status))
                        return status;
        }
        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, curr_val);
        return status;
}

static mc_status_t irgen_unary_operator(struct irgen_context *gen, 
                                        struct pt_node *unary_op,
                                        struct pt_node *cast_expr)
{
        mc_status_t status;
        struct ir_object *result = irgen_obj_get(gen);
        assert(pt_node_sym_cmp(unary_op, psym_unary_operator));
        struct ir_object *cast_expr_val = irgen_obj_create(gen, cast_expr);
        status = irgen_cast_expression(gen, cast_expr);
        if (!MC_SUCC(status))
                return status;
        irgen_value_set(gen, &result->val);
        switch ((int)pt_node_child_first(unary_op)->sym) {
                case PARSER_PUNCTUATOR(punc_bit_and):
                        status = irgen_obj_addr_of(gen, cast_expr_val);
                        break;
                case PARSER_PUNCTUATOR(punc_mul):
                        status = irgen_obj_deref(gen, cast_expr_val);
                        break;
                case PARSER_PUNCTUATOR(punc_add):
                        status = MC_OK;
                        break;
                case PARSER_PUNCTUATOR(punc_sub):
                        status = irgen_obj_neg(gen, cast_expr_val);
                        break;
                case PARSER_PUNCTUATOR(punc_tilde):
                        status = irgen_obj_not(gen, cast_expr_val);
                        break;
                case PARSER_PUNCTUATOR(punc_exclamation):
                        status = irgen_obj_log_not(gen, cast_expr_val);
                        break;
                default:
                        status = MC_IRGEN_ERROR;
                        MC_DBG(MC_CRIT, "unexpected unary operator");
        }
        return status;
}

static mc_status_t irgen_unary_expression(struct irgen_context *gen, 
                                          struct pt_node *stmt)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(stmt);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *last_expr = pt_node_child_last(stmt);

        struct ir_object *expr_val = irgen_obj_create(gen, last_expr);
        if (pt_node_sym_cmp(last_expr, psym_postfix_expression)) {
                status = irgen_postfix_expression(gen, last_expr);
        } else if (pt_node_sym_cmp(last_expr, psym_cast_expression)) {
                struct pt_node *unary_op = pt_node_child_number(stmt, 
                        --child_count);
                status = irgen_unary_operator(gen, unary_op, last_expr);
        } else {
                child_count -= 1;
                assert(pt_node_sym_cmp(pt_node_child_number(stmt, 
                        child_count), PARSER_KEYWORD(keyw_sizeof)));
                assert(pt_node_sym_cmp(last_expr, psym_type_name));
                status = irgen_obj_sizeof_tname(gen, last_expr);
        }

        if (!MC_SUCC(status))
                return status;

        for (uint16_t i_node = child_count - 1; i_node > 0; i_node--) {
                last_expr = pt_node_child_number(stmt, i_node); 
                if (pt_node_sym_cmp(last_expr, psym_type_name)) {
                        status = irgen_obj_sizeof(gen, expr_val);
                } else {
                        struct pt_node *prefix_op = pt_node_child_number(stmt, 
                                i_node);
                        if (pt_node_sym_cmp(prefix_op, 
                                PARSER_PUNCTUATOR(punc_increment))) {
                                status = irgen_obj_inc(gen, expr_val);
                        } else {
                                status = irgen_obj_dec(gen, expr_val);
                        }
                }
        }
        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, expr_val);
        return status;
}

static mc_status_t irgen_cast_expression(struct irgen_context *gen, 
                                         struct pt_node *stmt)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(stmt);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *last_expr = pt_node_child_last(stmt);
        struct ir_object *cast_expr_val = irgen_obj_create(gen, last_expr);
        status = irgen_unary_expression(gen, last_expr);
        if (!MC_SUCC(status))
                return status;

        for (uint16_t i_node = child_count - 1; i_node > 0; i_node--) {
                struct pt_node *cast_type = pt_node_child_number(stmt, i_node);
                status = irgen_obj_type_cast(gen, cast_expr_val, cast_type);
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, cast_expr_val);
        return status;
}

static mc_status_t irgen_multiplicative_expression(struct irgen_context *gen, 
                                                   struct pt_node *stmt)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(stmt);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(stmt);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_cast_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);

        for (uint16_t i_node = 2; i_node < child_count; i_node += 2) {
                struct pt_node *right_expr = pt_node_child_number(stmt, 
                        i_node + 1);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_cast_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!ir_obj_is_arithmetic(left_expr_val) 
                        || !ir_obj_is_arithmetic(right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "operands must be arithmetic");
                }

                struct pt_node *mult_op = pt_node_child_number(stmt, i_node);
                irgen_value_set(gen, &left_expr_val->val);
                
                if (pt_node_sym_cmp(mult_op, PARSER_PUNCTUATOR(punc_mul))) {
                        status = irgen_obj_mul(gen, right_expr_val, 
                                left_expr_val);
                } else if (pt_node_sym_cmp(mult_op, 
                        PARSER_PUNCTUATOR(punc_forward_slash))) {
                        status = irgen_obj_div(gen, right_expr_val, 
                                left_expr_val);
                } else {
                        if (!ir_obj_is_integer(left_expr_val) 
                                || !ir_obj_is_integer(right_expr_val)) {
                                return IRGEN_ERROR(gen, right_expr, 
                                        "operands must be integer type");
                        }
                        status = irgen_obj_mod(gen, right_expr_val, 
                                left_expr_val);
                }
                if (!MC_SUCC(status))
                        return status;
        }
        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static _Bool irgen_additive_op_valid(struct ir_object *op1, struct ir_object *op2)
{
        if (ir_obj_is_arithmetic(op1) && ir_obj_is_arithmetic(op2))
                return true;
        if ((ir_obj_is_ptr_obj(op1) && ir_obj_is_integer(op2))
                || (ir_obj_is_integer(op1) && ir_obj_is_ptr_obj(op2)))
                return true;
        return false;
}

static mc_status_t irgen_additive_expression(struct irgen_context *gen, 
                                                                                         struct pt_node *stmt)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(stmt);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(stmt);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_multiplicative_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);

        for (uint16_t i_node = 2; i_node < child_count; i_node += 2) {
                struct pt_node *right_expr = pt_node_child_number(stmt, 
                        i_node + 1);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_multiplicative_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!irgen_additive_op_valid(left_expr_val, right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "operands must be arithmetic or pointer");
                }

                struct pt_node *add_op = pt_node_child_number(stmt, i_node);
                irgen_value_set(gen, &left_expr_val->val);
                
                if (pt_node_sym_cmp(add_op, PARSER_PUNCTUATOR(punc_add))) {
                        status = irgen_obj_add(gen, right_expr_val, 
                                left_expr_val);
                } else {
                        status = irgen_obj_sub(gen, right_expr_val, 
                                left_expr_val);
                }
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static mc_status_t irgen_shift_expression(struct irgen_context *gen, 
                                          struct pt_node *stmt)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(stmt);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(stmt);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_additive_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);

        for (uint16_t i_node = 2; i_node < child_count; i_node += 2) {
                struct pt_node *right_expr = pt_node_child_number(stmt, 
                        i_node + 1);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_additive_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!ir_obj_is_integer(left_expr_val) 
                || !ir_obj_is_integer(right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "operands must be integer type");
                }

                struct pt_node *shift_op = pt_node_child_number(stmt, i_node);
                irgen_value_set(gen, &left_expr_val->val);
                
                if (pt_node_sym_cmp(shift_op, PARSER_PUNCTUATOR(punc_shl))) {
                        status = irgen_obj_lshift(gen, right_expr_val, 
                                left_expr_val);
                } else {
                        status = irgen_obj_rshift(gen, right_expr_val, 
                                left_expr_val);
                }
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static inline _Bool irgen_relational_op_valid(struct ir_object *op1, 
                                              struct ir_object *op2)
{
        if (ir_obj_is_arithmetic(op1) 
                && ir_obj_is_arithmetic(op2))
                return true;
        if (ir_obj_ptr_compat(op1, op2)) {
                if ((ir_obj_is_ptr_obj(op1) && ir_obj_is_ptr_obj(op2))
                        || (ir_obj_is_ptr_incompl(op1) 
                                && ir_obj_is_ptr_incompl(op2)))
                return true;
        }
        return false;
}

static mc_status_t irgen_relational_expression(struct irgen_context *gen, 
                                               struct pt_node *expr)
{

        mc_status_t status;
        uint16_t child_count = pt_node_child_count(expr);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(expr);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_shift_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);
        
        for (uint16_t i_node = 2; i_node < child_count; i_node += 2) {
                struct pt_node *right_expr = pt_node_child_number(expr, 
                        i_node + 1);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_shift_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!irgen_relational_op_valid(left_expr_val, 
                        right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "invalid operand types for relational expr");
                }

                struct pt_node *rel_op = pt_node_child_number(expr, i_node);
                irgen_value_set(gen, &left_expr_val->val);
                
                if (pt_node_sym_cmp(rel_op, PARSER_PUNCTUATOR(punc_less))) {
                        status = irgen_obj_lt(gen, right_expr_val, 
                                left_expr_val);
                } else if (pt_node_sym_cmp(rel_op, 
                        PARSER_PUNCTUATOR(punc_greater))) {
                        status = irgen_obj_gt(gen, right_expr_val, 
                                left_expr_val);
                } else if (pt_node_sym_cmp(rel_op, 
                        PARSER_PUNCTUATOR(punc_less_eq))) {
                        status = irgen_obj_le(gen, right_expr_val, 
                                left_expr_val);
                } else {
                        status = irgen_obj_ge(gen, right_expr_val, 
                                left_expr_val);
                }
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static inline _Bool irgen_equality_op_valid(struct ir_object *op1, 
                                            struct ir_object *op2)
{
        if (ir_obj_is_arithmetic(op1) 
                && ir_obj_is_arithmetic(op2))
                return true;
        if (ir_obj_ptr_compat(op1, op2))
                return true;
        if ((ir_obj_is_ptr_obj(op1) || ir_obj_is_ptr_incompl(op1))
                && ir_obj_is_ptr_void(op2))
                return true;
        if ((ir_obj_is_ptr_obj(op2) || ir_obj_is_ptr_incompl(op2))
                && ir_obj_is_ptr_void(op1))
                return true;
        if (ir_obj_is_pointer(op2) && ir_obj_is_nullptr(op1))
                return true;
        if (ir_obj_is_pointer(op2) && ir_obj_is_nullptr(op1))
                return true;
        return false;
}


static mc_status_t irgen_equality_expression(struct irgen_context *gen, 
                                             struct pt_node *expr)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(expr);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(expr);
        left_expr_val = irgen_obj_create(gen, 
                left_expr);
        status = irgen_relational_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);
        
        for (uint16_t i_node = 2; i_node < child_count; i_node += 2) {
                struct pt_node *right_expr = pt_node_child_number(expr, 
                        i_node + 1);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_relational_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!irgen_equality_op_valid(left_expr_val, right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "invalid operand types for equality expr");
                }

                struct pt_node *eq_op = pt_node_child_number(expr, i_node);
                irgen_value_set(gen, &left_expr_val->val);
                if (pt_node_sym_cmp(eq_op, PARSER_PUNCTUATOR(punc_equal))) {
                        status = irgen_obj_eq(gen, right_expr_val, 
                                left_expr_val);
                } else {
                        status = irgen_obj_neq(gen, right_expr_val, 
                                left_expr_val);
                }
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static mc_status_t irgen_and_expression(struct irgen_context *gen, 
                                        struct pt_node *expr)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(expr);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(expr);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_equality_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);
        
        for (uint16_t i_node = 2; i_node < child_count; i_node++) {
                struct pt_node *right_expr = pt_node_child_number(expr, i_node);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_equality_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!ir_obj_is_integer(left_expr_val) 
                        || !ir_obj_is_integer(right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "each of the operands shall have integer type");
                }

                irgen_value_set(gen, &left_expr_val->val);
                status = irgen_obj_and(gen, right_expr_val, left_expr_val);
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static mc_status_t irgen_exclusive_or_expression(struct irgen_context *gen, 
                                                 struct pt_node *expr)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(expr);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(expr);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_and_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);
        
        for (uint16_t i_node = 2; i_node < child_count; i_node++) {
                struct pt_node *right_expr = pt_node_child_number(expr, i_node);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_and_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!ir_obj_is_integer(left_expr_val) 
                        || !ir_obj_is_integer(right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "each of the operands shall have integer type");
                }

                irgen_value_set(gen, &left_expr_val->val);
                status = irgen_obj_xor(gen, right_expr_val, left_expr_val);
                if (!MC_SUCC(status))
                        return status;
        }
        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static mc_status_t irgen_inclusive_or_expression(struct irgen_context *gen, 
                                                 struct pt_node *expr)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(expr);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(expr);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_exclusive_or_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);
        
        for (uint16_t i_node = 2; i_node < child_count; i_node++) {
                struct pt_node *right_expr = pt_node_child_number(expr, i_node);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_exclusive_or_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!ir_obj_is_integer(left_expr_val) 
                        || !ir_obj_is_integer(right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr, 
                                "each of the operands shall have integer type");
                }

                irgen_value_set(gen, &left_expr_val->val);
                status = irgen_obj_or(gen, right_expr_val, left_expr_val);
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static mc_status_t irgen_logical_and_expression(struct irgen_context *gen, 
                                                struct pt_node *expr)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(expr);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(expr);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_inclusive_or_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);
        
        for (uint16_t i_node = 2; i_node < child_count; i_node++) {
                struct pt_node *right_expr = pt_node_child_number(expr, i_node);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_inclusive_or_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!ir_obj_is_scalar(left_expr_val) 
                        || !ir_obj_is_scalar(right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr,
                                "each of the operands shall have scalar type");
                }

                irgen_value_set(gen, &left_expr_val->val);
                status = irgen_obj_log_and(gen, right_expr_val, left_expr_val);
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static mc_status_t irgen_logical_or_expression(struct irgen_context *gen, 
                                               struct pt_node *expr)
{
        mc_status_t status;
        uint16_t child_count = pt_node_child_count(expr);
        struct ir_object *result = irgen_obj_get(gen);
        struct pt_node *left_expr;
        struct ir_object *left_expr_val;
        struct ir_object *right_expr_val;

        left_expr = pt_node_child_first(expr);
        left_expr_val = irgen_obj_create(gen, left_expr);
        status = irgen_logical_and_expression(gen, left_expr);
        if (!MC_SUCC(status))
                return status;

        if (child_count != 1)
                right_expr_val = irgen_obj_create(gen, NULL);
        
        for (uint16_t i_node = 2; i_node < child_count; i_node++) {
                struct pt_node *right_expr = pt_node_child_number(expr, i_node);
                irgen_value_set(gen, &right_expr_val->val);
                status = irgen_logical_and_expression(gen, right_expr);
                if (!MC_SUCC(status))
                        return status;

                if (!ir_obj_is_scalar(left_expr_val) 
                        || !ir_obj_is_scalar(right_expr_val)) {
                        return IRGEN_ERROR(gen, right_expr,
                                "each of the operands shall have scalar type");
                }

                irgen_value_set(gen, &left_expr_val->val);
                status = irgen_obj_log_or(gen, right_expr_val, left_expr_val);
                if (!MC_SUCC(status))
                        return status;
        }

        irgen_value_set(gen, &result->val);
        status = irgen_obj_move_single(gen, left_expr_val);
        return status;
}

static mc_status_t irgen_conditional_expression_compat(struct irgen_context *gen,
                                                       struct pt_node *node,
                                                       struct ir_object *obj1,
                                                       struct ir_object *obj2)
{
        struct ir_value *val1 = ir_obj_value_get(obj1);
        struct ir_value *val2 = ir_obj_value_get(obj2);
        if (ir_obj_is_arithmetic(obj1) && ir_obj_is_arithmetic(obj2))
                return true;
        if (ir_obj_compat_struct_or_union(obj1, obj2))
                return true;
        if (val1->type == ir_value_void && val2->type == ir_value_void)
                return true;
        if (ir_obj_ptr_compat(obj1, obj2))
                return true;
        if (ir_obj_is_pointer(obj1) && ir_obj_is_nullptr(obj2))
                return true;
        if (ir_obj_is_pointer(obj2) && ir_obj_is_nullptr(obj1))
                return true;
        if (ir_obj_is_ptr_obj(obj1) && ir_obj_is_ptr_void(obj2))
                return true;
        if (ir_obj_is_ptr_obj(obj2) && ir_obj_is_ptr_void(obj1))
                return true;
        return IRGEN_ERROR(gen, node, "types not compatible");
}

static mc_status_t irgen_conditional_expression(struct irgen_context *gen, 
                                                struct pt_node *stmt)
{
        mc_status_t status;
        struct ir_object *result = irgen_obj_get(gen);
        /* logical-or-expr should be scalar */
        struct pt_node *node_log_or_expr = pt_node_child_first(stmt);
        struct ir_object *log_expr_val = irgen_obj_create(gen, node_log_or_expr);
        status = irgen_logical_or_expression(gen, node_log_or_expr);
        if (!MC_SUCC(status))
                return status;

        assert(log_expr_val->type != ir_object_scalar);

        if (pt_node_child_count(stmt) != 1) {
                struct pt_node *node_expr = pt_node_child_number(stmt, 2);
                struct ir_object *expr_val = irgen_obj_create(gen, node_expr);
                status = irgen_expression(gen, node_expr);
                if (!MC_SUCC(status))
                        return status;

                struct pt_node *node_cond_expr = pt_node_child_number(stmt, 3);
                struct ir_object *cond_expr_val = irgen_obj_create(gen, 
                        node_cond_expr);
                status = irgen_conditional_expression(gen, node_cond_expr);
                if (!MC_SUCC(status))
                        return status;

                status = irgen_conditional_expression_compat(gen, stmt, 
                        expr_val, cond_expr_val);
                if (!MC_SUCC(status))
                        return status;

                /* set target value */
                irgen_value_set(gen, &result->val);
                status = irgen_obj_move_cond(gen, log_expr_val, expr_val, result);
        } else {
                irgen_value_set(gen, &result->val);
                status = irgen_obj_move_single(gen, log_expr_val);
        }
        return status;
}

static mc_status_t irgen_constant_expression(struct irgen_context *gen, 
                                             struct pt_node *stmt)
{
        mc_status_t status;

        assert(irgen_value_type(gen) == ir_value_object);
        struct ir_object *val_c_expr = irgen_obj_get(gen);

        status = irgen_conditional_expression(gen, pt_node_child_first(stmt));
        if (!MC_SUCC(status))
                return status;

        if (ir_obj_const_eval(val_c_expr))
                status = IRGEN_ERROR(gen, stmt, "expected const value");

        return status;
}

static mc_status_t irgen_labeled_statement_label(struct irgen_context *gen, 
                                                 struct pt_node *stmt)
{
        mc_status_t status = MC_PARSE_ERROR;
        struct label *lbl = irgen_get_label(gen, stmt);
        assert(lbl != NULL && lbl->block == NULL);

        if (!irgen_bb_empty(gen))
                irgen_bb_create(gen, stmt);
        lbl->block = irgen_bb_get(gen);

        status = irgen_statement(gen, pt_node_child_last(stmt));
        if (!MC_SUCC(status))
                return status;
                        
        return status;
}

struct switch_label_set*
irgen_switch_label_set_get(struct irgen_context *gen, struct pt_node *sw_node)
{
        if (sw_node->node_value.abstract_value != NULL) 
                return sw_node->node_value.abstract_value;

        struct switch_label_set* set = calloc(1, 
                sizeof(struct switch_label_set));
        stack_push(&gen->switch_sets, &set);
        sw_node->node_value.abstract_value = set;

        set->entries_capacity = SWITCH_LABEL_ENTRIES_COUNT;
        set->entries = calloc(set->entries_capacity, 
                sizeof(struct switch_label_entry));
        set->entries_count = 0;

        return set;
}

static inline 
struct switch_label_entry *
irgen_switch_label_set_entry(struct switch_label_set *set)
{
        if (set->entries_count >= set->entries_capacity) {
                set->entries_capacity *= 2;
                set->entries = realloc(set->entries, set->entries_capacity *
                        sizeof(struct switch_label_entry));
        }
        return &set->entries[set->entries_count++];
}

void irgen_switch_label_set_free(struct switch_label_set *set)
{
        free(set->entries);
        free(set);
}

/* from current default-case statement seek for a last switch statement,
 * current default-case labels belongs to
 * TODO: we may also pre-save it earlier for faster access */
static struct pt_node *irgen_labeled_statement_last_switch(struct pt_node *stmt)
{
        assert(stmt->sym == psym_labeled_statement);
        /* if stmt will went to NULL, we expect just crash */
        while (stmt->sym != psym_selection_statement || 
                pt_node_child_first(stmt)->sym != PARSER_KEYWORD(keyw_switch))
                stmt = stmt->parent;
        return stmt;
}

static mc_status_t irgen_labeled_statement_case(struct irgen_context *gen, 
                                                struct pt_node *stmt)
{
        mc_status_t status;
        struct ir_object *expr_val;

        /* add new basic block, if needed */
        if (!irgen_bb_empty(gen))
                irgen_bb_create(gen, stmt);
                
        struct basic_block *bb_stmt = irgen_bb_get(gen);
        status = irgen_statement(gen, pt_node_child_last(stmt));
        if (!MC_SUCC(status))
                return status;

        /* integer constant expression type here */
        struct pt_node *node_c_expr = pt_node_child_number(stmt, 2);
        irgen_obj_create(gen, node_c_expr);
        status = irgen_constant_expression(gen, node_c_expr);
        if (!MC_SUCC(status))
               return status; 
        expr_val = irgen_obj_get(gen);
        assert(expr_val->type == ir_object_scalar);

        struct pt_node *sw_node = irgen_labeled_statement_last_switch(stmt);
        struct switch_label_set *lbl_set 
                = irgen_switch_label_set_get(gen, sw_node);
        struct switch_label_entry *entry 
                = irgen_switch_label_set_entry(lbl_set);
        entry->block = bb_stmt;
        entry->val = expr_val;

        return status;
}

static mc_status_t irgen_labeled_statement_default(struct irgen_context *gen, 
                                                   struct pt_node *stmt)
{
        mc_status_t status;

        /* add new basic block, if needed */
        if (!irgen_bb_empty(gen))
                irgen_bb_create(gen, stmt);

        status = irgen_statement(gen, pt_node_child_last(stmt));
        if (!MC_SUCC(status))
                return status;

        struct pt_node *sw_node = irgen_labeled_statement_last_switch(stmt);
        struct switch_label_set *lbl_set 
                = irgen_switch_label_set_get(gen, sw_node);
        struct switch_label_entry *entry 
                = irgen_switch_label_set_entry(lbl_set);
        entry->block = irgen_bb_get(gen);
        entry->val = NULL;

        return status;
}

static mc_status_t irgen_labeled_statement(struct irgen_context *gen, 
                                           struct pt_node *stmt)
{
        mc_status_t status = MC_OK;
        struct pt_node *first = pt_node_child_first(stmt);
        switch ((int)first->sym) {
                case psym_identifier:
                        status = irgen_labeled_statement_label(gen, stmt);
                        break;
                case PARSER_KEYWORD(keyw_case):
                        status = irgen_labeled_statement_case(gen, stmt);
                        break;
                case PARSER_KEYWORD(keyw_default):
                        status = irgen_labeled_statement_default(gen, stmt);
                        break;
                default:
                        status = MC_PARSE_ERROR;
        }
        return status;
}

static mc_status_t irgen_expression_statement(struct irgen_context *gen, 
                                              struct pt_node *stmt)
{
        UNUSED(gen);
        UNUSED(stmt);
        return MC_OK;
}

static mc_status_t irgen_selection_statement(struct irgen_context *gen, 
                                             struct pt_node *stmt)
{
        UNUSED(gen);
        UNUSED(stmt);
        return MC_OK;
}

static mc_status_t irgen_iteration_statement(struct irgen_context *gen, 
                                             struct pt_node *stmt)
{
        UNUSED(gen);
        UNUSED(stmt);
        return MC_OK;
}

static mc_status_t irgen_jump_statement(struct irgen_context *gen, 
                                        struct pt_node *stmt)
{
        UNUSED(gen);
        UNUSED(stmt);
        return MC_OK;
}

static mc_status_t irgen_statement(struct irgen_context *gen, 
                                   struct pt_node *stmt)
{
        mc_status_t status = MC_OK;
        struct pt_node *conc_stmt = pt_node_child_first(stmt);
        switch (conc_stmt->sym) {
                case psym_labeled_statement:
                        status = irgen_labeled_statement(gen, conc_stmt);
                        break;
                case psym_compound_statement:
                        status = irgen_compound_statement(gen, conc_stmt);
                        break;
                case psym_expression_statement:
                        status = irgen_expression_statement(gen, conc_stmt);
                        break;
                case psym_selection_statement:
                        status = irgen_selection_statement(gen, conc_stmt);
                        break;
                case psym_iteration_statement:
                        status = irgen_iteration_statement(gen, conc_stmt);
                        break;
                case psym_jump_statement:
                        status = irgen_jump_statement(gen, conc_stmt);
                        break;
                default:
                        status = MC_PARSE_ERROR;
                        break;
        }
        return status;
}

static mc_status_t irgen_block_item(struct irgen_context *gen, 
                                    struct pt_node *blk_itm)
{
        mc_status_t status = MC_PARSE_ERROR;

        struct pt_node *item = pt_node_child_first(blk_itm);
        if (item->sym == psym_declaration) {
                /* we do nothig, we will allocate var when used */
                status = MC_OK;
        } else if (item->sym == psym_statement) {
                status = irgen_statement(gen, item);
        }
        return status;
}

static mc_status_t irgen_block_item_list(struct irgen_context *gen, 
                                         struct pt_node *blk_itm_lst)
{
        mc_status_t status = MC_PARSE_ERROR;
        AST_FOREACH_CHILD(blk_itm_lst) {
                struct pt_node *blk_itm = (struct pt_node *)entry;
                status = irgen_block_item(gen, blk_itm);
                if (!MC_SUCC(status))
                        break;
        }
        return status; 
}

static mc_status_t irgen_compound_statement(struct irgen_context *gen, 
                                            struct pt_node *comp_stmt)
{
        mc_status_t status = MC_OK;
        assert(comp_stmt->sym == psym_compound_statement);
        if (pt_node_child_count(comp_stmt) != 0) {
                struct pt_node *blk_itm_lst = pt_node_child_first(comp_stmt);
                status = irgen_block_item_list(gen, blk_itm_lst);
        }
        return status; 
}

static mc_status_t irgen_function_definition(struct irgen_context *gen, 
                                            struct pt_node *func_def)
{
        mc_status_t status = MC_PARSE_ERROR;
        /* TODO: check func def constraints 6.9.1 */

        irgen_bb_create(gen, func_def);
        status = irgen_compound_statement(gen, pt_node_child_last(func_def));

        return status;
}

static mc_status_t irgen_external_declaration(struct irgen_context *gen, 
                                              struct pt_node *ex_decl)
{
        mc_status_t status = MC_PARSE_ERROR;
        assert(ex_decl->sym == psym_external_declaration);
        struct pt_node *decl = pt_node_child_first(ex_decl);
        switch (decl->sym) {
                case psym_function_definition:
                        irgen_function_create(gen, decl);
                        status = irgen_function_definition(gen, decl);
                        break;
                case psym_declaration:
                        /* we already done with it in parser.c */
                        status = MC_OK;
                        break;
                default:
                        MC_DBG(MC_CRIT, "unexpected node type");
                        break;
        }
        return status;
}

static struct irgen_translation_ops irgen_translation_ops = {
        .external_declaration = irgen_external_declaration,
        .function_definition = irgen_function_definition,

        .statement = irgen_statement,
        .compound_statement = irgen_compound_statement,
        .labeled_statement = irgen_labeled_statement,
        .expression_statement = irgen_expression_statement,
        .selection_statement = irgen_selection_statement,
        .iteration_statement = irgen_iteration_statement,
        .jump_statement = irgen_jump_statement,

        .expression = irgen_expression,
        .assignment_expression = NULL,
        .conditional_expression = irgen_conditional_expression,
        .logical_or_expression = irgen_logical_or_expression,
        .logical_and_expression = irgen_logical_and_expression,
        .inclusive_or_expression = irgen_inclusive_or_expression,
        .exclusive_or_expression = irgen_exclusive_or_expression,
        .and_expression = irgen_and_expression,
        .equality_expression = irgen_equality_expression,
        .relational_expression = irgen_relational_expression,
        .shift_expression = irgen_shift_expression,
        .additive_expression = irgen_additive_expression,
        .multiplicative_expression = irgen_multiplicative_expression,
        .cast_expression = irgen_cast_expression,
        .unary_expression = irgen_unary_expression,
        .postfix_expression = irgen_postfix_expression,
        .primary_expression = irgen_primary_expression,

        .initializer = NULL,
        .initializer_list = irgen_initializer_list
};

void _irgen_translation_ops_init(struct irgen_context *gen)
{
        gen->ops = &irgen_translation_ops;
}