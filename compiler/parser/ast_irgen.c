#include <assert.h>
#include <ir/gen.h>
#include <ir.h>
#include <parser/ast.h>

static mc_status_t irgen_compound_statement(struct irgen_context *gen, 
                                            struct pt_node *comp_stmt);
static mc_status_t irgen_statement(struct irgen_context *gen, 
                                   struct pt_node *stmt);

static mc_status_t irgen_expression(struct irgen_context *gen, 
                                    struct pt_node *stmt)
{
        UNUSED(gen);
        UNUSED(stmt);
        return MC_FAIL;
}

static mc_status_t irgen_equality_expression(struct irgen_context *gen, 
                                             struct pt_node *expr)
{
        UNUSED(gen);
        UNUSED(expr);
        return MC_FAIL;
}

static mc_status_t irgen_and_expression(struct irgen_context *gen, 
                                        struct pt_node *expr)
{
        mc_status_t status;
        struct lvalue *result = irgen_lvalue_get(gen);

        struct pt_node *node_eq_expr = pt_node_child_last(expr);
        struct lvalue *eq_expr_val = irgen_lvalue_create(gen, node_eq_expr);
        status = irgen_equality_expression(gen, node_eq_expr);
        if (!MC_SUCC(status))
                return status;
        assert(eq_expr_val->type == lvalue_scalar);

        if (pt_node_child_count(expr) == 2) {
                struct pt_node *node_and_expr = pt_node_child_first(expr); 
                struct lvalue *and_expr_val = irgen_lvalue_create(gen, 
                        node_and_expr);
                status = irgen_and_expression(gen, node_and_expr);
                if (!MC_SUCC(status))
                        return status;
                assert(and_expr_val->type == lvalue_scalar);

                struct lvalue *expr_val = irgen_lvalue_create(gen, expr);
                status = irgen_lvalue_and(gen, eq_expr_val, and_expr_val);
                eq_expr_val = expr_val;
        }
        irgen_value_set(gen, &result->val);
        status = irgen_lvalue_move_single(gen, eq_expr_val);
        return status;
}

static mc_status_t irgen_exclusive_or_expression(struct irgen_context *gen, 
                                                 struct pt_node *expr)
{
        mc_status_t status;
        struct lvalue *result = irgen_lvalue_get(gen);

        struct pt_node *node_and_expr = pt_node_child_last(expr);
        struct lvalue *and_expr_val = irgen_lvalue_create(gen, node_and_expr);
        status =  irgen_and_expression(gen, node_and_expr);
        if (!MC_SUCC(status))
                return status;
        assert(and_expr_val->type == lvalue_scalar);

        if (pt_node_child_count(expr) == 2) {
                struct pt_node *node_xor_expr = pt_node_child_first(expr);
                struct lvalue *xor_expr_val = irgen_lvalue_create(gen, 
                        node_xor_expr);
                status = irgen_exclusive_or_expression(gen, node_xor_expr);
                if (!MC_SUCC(status))
                        return status;
                assert(xor_expr_val->type == lvalue_scalar);

                struct lvalue *expr_val = irgen_lvalue_create(gen, expr);
                status = irgen_lvalue_xor(gen, and_expr_val, xor_expr_val);
                and_expr_val = expr_val;
        }
        irgen_value_set(gen, &result->val);
        status = irgen_lvalue_move_single(gen, and_expr_val);
        return status;
}

static mc_status_t irgen_inclusive_or_expression(struct irgen_context *gen, 
                                                 struct pt_node *expr)
{
        mc_status_t status;
        struct lvalue *result = irgen_lvalue_get(gen);

        struct pt_node *node_exclusive_or_expr = pt_node_child_last(expr);
        struct lvalue *exclusive_or_expr_val = irgen_lvalue_create(gen, 
                node_exclusive_or_expr);
        status = irgen_exclusive_or_expression(gen, node_exclusive_or_expr);
        if (!MC_SUCC(status))
                return status;
        assert(exclusive_or_expr_val->type == lvalue_scalar);

        if (pt_node_child_count(expr) == 2) {
                struct pt_node *node_incl_or_expr = pt_node_child_first(expr);
                struct lvalue *incl_or_expr_val = irgen_lvalue_create(gen, 
                        node_incl_or_expr);
                status = irgen_inclusive_or_expression(gen, node_incl_or_expr);
                if (!MC_SUCC(status))
                        return status;
                assert(incl_or_expr_val->type == lvalue_scalar);

                struct lvalue *expr_val = irgen_lvalue_create(gen, expr);
                status = irgen_lvalue_or(gen, exclusive_or_expr_val, 
                        incl_or_expr_val);
                exclusive_or_expr_val = expr_val;
        }
        irgen_value_set(gen, &result->val);
        status = irgen_lvalue_move_single(gen, exclusive_or_expr_val);
        return status;
}

static mc_status_t irgen_logical_and_expression(struct irgen_context *gen, 
                                                struct pt_node *expr)
{
        mc_status_t status;
        struct lvalue *result = irgen_lvalue_get(gen);

        struct pt_node *node_inc_expr = pt_node_child_last(expr);
        struct lvalue *inc_expr_val = irgen_lvalue_create(gen, node_inc_expr);
        status = irgen_inclusive_or_expression(gen, node_inc_expr);
        if (!MC_SUCC(status))
                return status;
        assert(inc_expr_val ->type == lvalue_scalar);

        if (pt_node_child_count(expr) == 2) {
                struct pt_node *node_log_and_expr = pt_node_child_first(expr);
                struct lvalue *log_expr_val = irgen_lvalue_create(gen, 
                        node_log_and_expr);
                status = irgen_logical_and_expression(gen, node_log_and_expr);
                if (!MC_SUCC(status))
                        return status;
                assert(log_expr_val->type == lvalue_scalar);

                struct lvalue *expr_val = irgen_lvalue_create(gen, expr);
                status = irgen_lvalue_log_and(gen, inc_expr_val, log_expr_val);
                inc_expr_val = expr_val; 
        }
        irgen_value_set(gen, &result->val);
        status = irgen_lvalue_move_single(gen, inc_expr_val);
        return status;
}

static mc_status_t irgen_logical_or_expression(struct irgen_context *gen, 
                                               struct pt_node *expr)
{
        mc_status_t status;
        struct lvalue *result = irgen_lvalue_get(gen);

        struct pt_node *node_and_expr = pt_node_child_last(expr);
        struct lvalue *and_expr_val = irgen_lvalue_create(gen, node_and_expr);
        status = irgen_logical_and_expression(gen, node_and_expr);
        if (!MC_SUCC(status))
                return status;
        assert(and_expr_val->type == lvalue_scalar);

        if (pt_node_child_count(expr) == 2) {
                struct pt_node *node_log_or_expr = pt_node_child_first(expr);
                struct lvalue *log_expr_val = irgen_lvalue_create(gen, 
                        node_log_or_expr);
                status = irgen_logical_or_expression(gen, node_log_or_expr);
                if (!MC_SUCC(status))
                        return status;
                assert(log_expr_val->type == lvalue_scalar);

                struct lvalue *expr_val = irgen_lvalue_create(gen, expr);
                status = irgen_lvalue_log_or(gen, and_expr_val, log_expr_val);
                and_expr_val = expr_val;
        }
        irgen_value_set(gen, &result->val);
        status = irgen_lvalue_move_single(gen, and_expr_val);
        return status;
}

static mc_status_t irgen_conditional_expression_compat
                                             (struct irgen_context *gen, 
                                             struct pt_node *node,
                                             struct lvalue *val1, 
                                             struct lvalue *val2)
{
        if (ir_lvalue_is_arithmetic(val1) && ir_lvalue_is_arithmetic(val2))
                return true;
        if (ir_lvalue_compat_struct_or_union(val1, val2)) 
                return true;
        if (val1->type == lvalue_void && val2->type == lvalue_void) 
                return true;
        if (ir_lvalue_ptr_compat(val1, val2))
                return true;
        if (ir_lvalue_is_pointer(val1) && ir_lvalue_is_nullptr(val2))
                return true;
        if (ir_lvalue_is_pointer(val2) && ir_lvalue_is_nullptr(val1))
                return true;
        if (ir_lvalue_is_ptr_obj(val1) && ir_lvalue_is_ptr_void(val2))
                return true;
        if (ir_lvalue_is_ptr_obj(val2) && ir_lvalue_is_ptr_void(val1))
                return true;
        return IRGEN_ERROR(gen, node, "types not compatible");
}

static mc_status_t irgen_conditional_expression(struct irgen_context *gen, 
                                                struct pt_node *stmt)
{
        mc_status_t status;
        struct lvalue *result = irgen_lvalue_get(gen);
        /* logical-or-expr should be scalar */
        struct pt_node *node_log_or_expr = pt_node_child_first(stmt);
        struct lvalue *log_expr_val = irgen_lvalue_create(gen, node_log_or_expr);
        status = irgen_logical_or_expression(gen, node_log_or_expr);
        if (!MC_SUCC(status))
                return status;

        assert(log_expr_val->type != lvalue_scalar);

        if (pt_node_child_count(stmt) != 1) {
                struct pt_node *node_expr = pt_node_child_number(stmt, 2);
                struct lvalue *expr_val = irgen_lvalue_create(gen, node_expr);
                status = irgen_expression(gen, node_expr);
                if (!MC_SUCC(status))
                        return status;

                struct pt_node *node_cond_expr = pt_node_child_number(stmt, 3);
                struct lvalue *cond_expr_val = irgen_lvalue_create(gen, 
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
                status = irgen_lvalue_move_cond(gen, log_expr_val, expr_val, 
                        result);
        } else {
                irgen_value_set(gen, &result->val);
                status = irgen_lvalue_move_single(gen, log_expr_val);
        }
        return status;
}

static mc_status_t irgen_constant_expression(struct irgen_context *gen, 
                                             struct pt_node *stmt)
{
        mc_status_t status;

        assert(irgen_value_type(gen) == ir_value_lvalue);
        struct lvalue *val_c_expr = irgen_lvalue_get(gen);

        status = irgen_conditional_expression(gen, pt_node_child_first(stmt));
        if (!MC_SUCC(status))
                return status;

        if (ir_lvalue_const_eval(val_c_expr))
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
        struct lvalue *expr_val;

        /* add new basic block, if needed */
        if (!irgen_bb_empty(gen))
                irgen_bb_create(gen, stmt);
                
        struct basic_block *bb_stmt = irgen_bb_get(gen);
        status = irgen_statement(gen, pt_node_child_last(stmt));
        if (!MC_SUCC(status))
                return status;

        /* integer constant expression type here */
        struct pt_node *node_c_expr = pt_node_child_number(stmt, 2);
        irgen_lvalue_create(gen, node_c_expr);
        status = irgen_constant_expression(gen, node_c_expr);
        if (!MC_SUCC(status))
               return status; 
        expr_val = irgen_lvalue_get(gen);
        assert(expr_val->type == lvalue_scalar);

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

mc_status_t irgen_translation_unit(struct irgen_context *gen, 
                                   struct pt_node *unit)
{
        mc_status_t status;
        assert(unit->sym == psym_translation_unit);

        AST_FOREACH_CHILD(unit) {
                struct pt_node *ext_decl = (struct pt_node *)entry;
                status = irgen_external_declaration(gen, ext_decl);
                if (!MC_SUCC(status))
                        goto fail;
        }
        
        return status;
fail:
        irgen_free(gen);
        return status;
}