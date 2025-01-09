#ifndef _AST_IRGEN_H_
#define _AST_IRGEN_H_
#include <stack.h>
#include <parser/ast.h>
#include <ir.h>

#define IRGEN_STACK_CAPACITY 10

struct irgen_context {
        struct parser *parser;
        struct module *mod;

        struct function *curr_func;

        /* the abstraction of value we are currently
         * working with. The main two examples may be
         * a basic block or some constant value */
        struct value *curr_value;
        struct stack switch_sets;

        uint32_t label_idx;

        mc_status_t (*error)(struct pt_node *node, const char *message);
};

#define IRGEN_ERROR(gen, node, message)                               \
        gen->error(node, message)

struct declaration *irgen_get_declaration(struct irgen_context *gen);

struct switch_label_entry {
        /* we heve block - we know where to jump, we have
         * value - we know when to jump */
        struct basic_block *block;
        /* NULL mean default */
        struct lvalue *val; 
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
struct lvalue *irgen_lvalue_create(struct irgen_context *gen, 
        struct pt_node *lval_node);

struct lvalue *irgen_scalar_create(struct irgen_context *gen, 
        struct scalar_var var);

/* constraints: @value_true/false may have first basic block in use, which will
 * lead to their initial value assignatioin, make sure associated with them 
 * code was not created multiple times, that will lead to undef behaviour 
 * @condition most be scalar type */
mc_status_t irgen_lvalue_move_cond(struct irgen_context *gen, 
                                   struct lvalue *condition, 
                                   struct lvalue *value_true, 
                                   struct lvalue *value_false);

mc_status_t irgen_lvalue_move_single(struct irgen_context *gen, 
                                     struct lvalue *val_mov);

mc_status_t irgen_lvalue_log_or(struct irgen_context *gen, 
                                struct lvalue *val1, 
                                struct lvalue *val2);

mc_status_t irgen_lvalue_log_and(struct irgen_context *gen, 
                                struct lvalue *val1, 
                                struct lvalue *val2);

mc_status_t irgen_lvalue_or(struct irgen_context *gen, 
                            struct lvalue *val1, 
                            struct lvalue *val2);
static inline 
enum ir_value_type 
irgen_value_type(struct irgen_context *gen)
{
        return gen->curr_value->type;
}

/* value of the last ir-gen operation is stored */
static inline void
irgen_value_set(struct irgen_context *gen, struct value *val)
{
        gen->curr_value = val;
}

static inline 
struct basic_block *
irgen_bb_get(struct irgen_context *gen)
{
        return ir_value_bb_get(gen->curr_value);
}

static inline 
struct lvalue *
irgen_lvalue_get(struct irgen_context *gen)
{
        return ir_lvalue_get(gen->curr_value);
}

static inline _Bool irgen_bb_empty(struct irgen_context *gen)
{
        if (gen->curr_value->type != ir_value_basic_block)
                return true;
        struct basic_block *bb = irgen_bb_get(gen);
        return ir_bb_empty(bb);
}

mc_status_t irgen_translation_unit(struct irgen_context *gen, 
                                   struct pt_node *unit);

#endif /* _AST_IRGEN_H_ */