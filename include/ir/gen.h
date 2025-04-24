#ifndef _AST_IRGEN_H_
#define _AST_IRGEN_H_
#include <stack.h>
#include <parser.h>
#include <ir.h>

#define IRGEN_STACK_CAPACITY 10

typedef mc_status_t (*irgen_error_t)(struct pt_node *node,
                                     const char *message);

struct irgen_context {
        struct parser *parser;
        struct ir_module *mod;

        struct function *curr_func;

        /* the abstraction of value we are currently
         * working with. The main two examples may be
         * a basic block or some constant value */
        struct ir_value *curr_value;
        struct stack switch_sets;

        const struct irgen_translation_ops *ops;
        irgen_error_t error;

        uint32_t label_idx;
};

typedef mc_status_t (*irgen_decl_func_t)(struct irgen_context *gen,
                                         struct pt_node *decl);
typedef mc_status_t (*irgen_stmt_func_t)(struct irgen_context *gen,
                                         struct pt_node *stmt);
typedef mc_status_t (*irgen_expr_func_t)(struct irgen_context *gen,
                                         struct pt_node *stmt);

struct irgen_translation_ops {
        irgen_decl_func_t external_declaration;
        irgen_decl_func_t function_definition;

        irgen_stmt_func_t statement;
        irgen_stmt_func_t compound_statement;
        irgen_stmt_func_t labeled_statement;
        irgen_stmt_func_t expression_statement;
        irgen_stmt_func_t selection_statement;
        irgen_stmt_func_t iteration_statement;
        irgen_stmt_func_t jump_statement;

        irgen_expr_func_t expression;
        irgen_expr_func_t assignment_expression;
        irgen_expr_func_t conditional_expression;
        irgen_expr_func_t logical_or_expression;
        irgen_expr_func_t logical_and_expression;
        irgen_expr_func_t inclusive_or_expression;
        irgen_expr_func_t exclusive_or_expression;
        irgen_expr_func_t and_expression;
        irgen_expr_func_t equality_expression;
        irgen_expr_func_t relational_expression;
        irgen_expr_func_t shift_expression;
        irgen_expr_func_t additive_expression;
        irgen_expr_func_t multiplicative_expression;
        irgen_expr_func_t cast_expression;
        irgen_expr_func_t unary_expression;
        irgen_expr_func_t postfix_expression;
        irgen_expr_func_t primary_expression;

        irgen_stmt_func_t initializer;
        irgen_stmt_func_t initializer_list;
};

void _irgen_translation_ops_init(struct irgen_context *gen);

#define IRGEN_ERROR(gen, node, message)                               \
        gen->error(node, message)

struct declaration *irgen_get_declaration(struct irgen_context *gen, 
                                          struct token *id,
                                          struct pt_node *scope);

struct switch_label_entry {
        /* we heve block - we know where to jump, we have
         * value - we know when to jump */
        struct basic_block *block;
        /* NULL mean default */
        struct ir_object *val; 
};

#define SWITCH_LABEL_ENTRIES_COUNT 5

struct switch_label_set {
        struct list_head *link; /* next processed switch */

        struct switch_label_entry *entries;
        size_t entries_count;
        size_t entries_capacity;
};

struct label *irgen_get_label(struct irgen_context *gen, struct pt_node *scope);

void irgen_switch_label_set_free(struct switch_label_set *set);

void irgen_init(struct irgen_context *gen, struct parser *ps);

void irgen_free(struct irgen_context *gen);

static inline
uint32_t irgen_label_index(struct irgen_context *gen)
{
        return gen->label_idx++;
}

void irgen_function_create(struct irgen_context *gen,
                           struct pt_node *func_def);

struct basic_block *irgen_bb_create(struct irgen_context *gen, 
        struct pt_node *source);

/* @lval_node ast node associated with this value definition */
struct ir_object *irgen_obj_create(struct irgen_context *gen, 
        struct pt_node *lval_node);

/* @lval_node ast node associated with this value definition */
struct ir_object *irgen_obj_create(struct irgen_context *gen, 
        struct pt_node *lval_node);

struct ir_object *irgen_scalar_create(struct irgen_context *gen, 
        struct ir_scalar var);

/* constraints: @value_true/false may have first basic block in use, which will
 * lead to their initial value assignatioin, make sure associated with them 
 * code was not created multiple times, that will lead to undef behaviour 
 * @condition most be scalar type */
mc_status_t irgen_obj_move_cond(struct irgen_context *gen, 
                               struct ir_object *condition, 
                               struct ir_object *value_true, 
                               struct ir_object *value_false);

mc_status_t irgen_obj_move_single(struct irgen_context *gen, 
                                 struct ir_object *val_mov);

mc_status_t irgen_obj_log_or(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2);

mc_status_t irgen_obj_log_and(struct irgen_context *gen, 
                             struct ir_object *val1, 
                             struct ir_object *val2);

mc_status_t irgen_obj_or(struct irgen_context *gen, 
                        struct ir_object *val1, 
                        struct ir_object *val2);

mc_status_t irgen_obj_xor(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_and(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_eq(struct irgen_context *gen, 
                        struct ir_object *val1, 
                        struct ir_object *val2);

mc_status_t irgen_obj_neq(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_lt(struct irgen_context *gen, 
                        struct ir_object *val1, 
                        struct ir_object *val2);

mc_status_t irgen_obj_gt(struct irgen_context *gen, 
                        struct ir_object *val1, 
                        struct ir_object *val2);

mc_status_t irgen_obj_le(struct irgen_context *gen, 
                        struct ir_object *val1, 
                        struct ir_object *val2);

mc_status_t irgen_obj_ge(struct irgen_context *gen, 
                        struct ir_object *val1, 
                        struct ir_object *val2);

mc_status_t irgen_obj_lshift(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2);

mc_status_t irgen_obj_rshift(struct irgen_context *gen, 
                            struct ir_object *val1, 
                            struct ir_object *val2);

mc_status_t irgen_obj_add(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_sub(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_inc(struct irgen_context *gen, 
                         struct ir_object *val);

mc_status_t irgen_obj_dec(struct irgen_context *gen, 
                         struct ir_object *val);

mc_status_t irgen_obj_mul(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_div(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_mod(struct irgen_context *gen, 
                         struct ir_object *val1, 
                         struct ir_object *val2);

mc_status_t irgen_obj_type_cast(struct irgen_context *gen, 
                               struct ir_object *val, 
                               struct pt_node *type);

mc_status_t irgen_obj_sizeof(struct irgen_context *gen, 
                            struct ir_object *val);

mc_status_t irgen_obj_sizeof_tname(struct irgen_context *gen, 
                                  struct pt_node *tnane);

mc_status_t irgen_obj_addr_of(struct irgen_context *gen, 
                             struct ir_object *val);

mc_status_t irgen_obj_deref(struct irgen_context *gen, 
                           struct ir_object *val);

mc_status_t irgen_obj_neg(struct irgen_context *gen, 
                         struct ir_object *val);

mc_status_t irgen_obj_not(struct irgen_context *gen, 
                         struct ir_object *val);

mc_status_t irgen_obj_log_not(struct irgen_context *gen, 
                             struct ir_object *val);

mc_status_t irgen_obj_struct_member_op(struct irgen_context *gen,
                                      struct ir_object *object,
                                      struct pt_node *member);

mc_status_t irgen_obj_struct_ptr_op(struct irgen_context *gen,
                                   struct ir_object *ptr_object,
                                   struct pt_node *member);

mc_status_t irgen_obj_array_index(struct irgen_context *gen,
                                 struct ir_object *array,
                                 struct ir_object *index);

static inline enum ir_value_type
irgen_value_type(struct irgen_context *gen)
{
        return gen->curr_value->type;
}

/* value of the last ir-gen operation is stored */
static inline void
irgen_value_set(struct irgen_context *gen, struct ir_value *val)
{
        gen->curr_value = val;
}

static inline void 
irgen_value_obj_set(struct irgen_context *gen, 
                                       struct ir_object *result)
{
        irgen_value_set(gen, &result->val);
}

static inline 
struct ir_value *
irgen_value_seek(struct irgen_context *gen, struct pt_node *id)
{
        return ir_function_value_seek(gen->curr_func, id);
}

static inline 
struct basic_block *
irgen_bb_get(struct irgen_context *gen)
{
        return ir_value_bb_get(gen->curr_value);
}

static inline 
struct ir_object *
irgen_obj_get(struct irgen_context *gen)
{
        return ir_obj_get(gen->curr_value);
}

static inline _Bool irgen_bb_empty(struct irgen_context *gen)
{
        if (gen->curr_value->type != ir_value_basic_block)
                return true;
        struct basic_block *bb = irgen_bb_get(gen);
        return ir_bb_empty(bb);
}

#endif /* _AST_IRGEN_H_ */