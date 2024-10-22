#include <stdlib.h>
#include <tools/fnv_1.h>
#include <ir.h>
#include <parser/ast.h>

#define IR_BASIC_BLOCK_LABLE_RADIX 10

static void ir_function_destroy(struct function *func);

struct module *ir_module_create()
{
        return calloc(1, sizeof(struct module));
}

void ir_module_destroy(struct module *module)
{
        assert(false);
        for (size_t i = 0; i < module->func_count; i++)
                ir_function_destroy(module->functions[i]);
        free(module);
}

struct basic_block *ir_bb_create(const char *prefix, uint32_t index)
{
        struct basic_block *bb = calloc(1, sizeof(struct basic_block));
        uint32_t pref_size = 0;
        if (prefix != NULL) {
                pref_size = strlen(prefix);
                strcat(bb->lable, prefix);
        }
        ultoa(index, bb->lable + pref_size, IR_BASIC_BLOCK_LABLE_RADIX);
        return bb;
}

static inline 
struct basic_block *
ir_bb_hlist_entry(struct hlist_entry *node)
{
        return container_of(node, struct basic_block, hlist);
}

static void ir_bb_free(struct hlist_entry *node)
{
        /* free instructions */
        assert(false);
        free(ir_bb_hlist_entry(node)); 
}

static hash_key_t ir_bb_hash(struct hlist_entry *node)
{
        return ir_bb_hlist_entry(node)->hash;
}

struct function *ir_function_create(struct module *module, 
                                    struct pt_node *func_def)
{
        struct function *func = calloc(1, sizeof(struct function));
        module->functions[++module->func_count] = func;

        struct pt_node *declarator = pt_node_child_number(func_def, 2);
        struct token *func_id = ast_declarator_id(declarator);

        func->name = func_id;
        struct hash_table_ops t_ops = {
                .free = ir_bb_free,
                .get_key = ir_bb_hash,
        };
        hash_init(&func->tbl, t_ops);
        return func;
}

void ir_function_block_add(struct function *func, struct basic_block *bb)
{
        if (func->entry == NULL)
                func->entry = bb;

        bb->hash = fnv_1_hash(bb->lable, strlen(bb->lable));
        hash_add(&func->tbl, &bb->hlist);
}

static void ir_function_destroy(struct function *func)
{
        hash_free(&func->tbl);
        free(func);
}
