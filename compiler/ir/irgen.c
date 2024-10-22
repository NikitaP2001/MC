#include <ir/irgen.h>

void irgen_init(struct irgen_context *gen)
{
        memset(gen, 0, sizeof(struct irgen_context));
        gen->mod = ir_module_create();
}

void irgen_free(struct irgen_context *gen)
{
        ir_module_destroy(gen->mod);
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
struct basic_block *
irgen_bb_create(struct irgen_context *gen, struct pt_node *source)
{
        struct basic_block *bb_old = gen->curr_block;
        const char *preffix = NULL;
        switch (source->sym) {
                case psym_function_definition:
                        preffix = "entry: ";
                        break;
                default:
                        preffix = "bb: ";
                        break;
        }

        struct basic_block *bb = ir_bb_create(preffix, irgen_label_index(gen));
        gen->curr_block = bb;
        ir_function_block_add(gen->curr_func, bb);
        return bb_old;
}