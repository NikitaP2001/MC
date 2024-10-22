#ifndef _AST_IRGEN_H_
#define _AST_IRGEN_H_
#include <stack.h>
#include <parser/ast.h>
#include <ir.h>

#define IRGEN_STACK_CAPACITY 10

struct irgen_context {
    struct module *mod;

    struct function *curr_func;

    struct basic_block *curr_block;

    uint32_t label_idx;
};

void irgen_init(struct irgen_context *gen);

void irgen_free(struct irgen_context *gen);

static inline
uint32_t irgen_label_index(struct irgen_context *gen)
{
        return gen->label_idx++;
}

void irgen_function_create(struct irgen_context *gen,
                           struct pt_node *func_def);

struct basic_block *
irgen_bb_create(struct irgen_context *gen, struct pt_node *source);

static inline void irgen_bb_restore(struct irgen_context *gen, 
                                    struct basic_block *old)
{
        assert(gen->curr_block != old);
        gen->curr_block = old;
}

#endif /* _AST_IRGEN_H_ */