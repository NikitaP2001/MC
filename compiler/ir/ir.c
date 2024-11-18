#include <stdlib.h>
#include <tools/fnv_1.h>
#include <ir.h>
#include <parser/ast.h>

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

struct function *ir_function_create(struct module *module, 
                                    struct pt_node *func_def)
{
        struct function *func = calloc(1, sizeof(struct function));
        module->functions[++module->func_count] = func;

        struct pt_node *declarator = pt_node_child_number(func_def, 2);
        struct token *func_id = ast_declarator_id(declarator);

        func->name = func_id;
        struct hash_table_ops t_ops = {
                .free = ir_value_free,
                .get_key = ir_value_hash,
        };
        hash_init(&func->var_tbl, t_ops);
        return func;
}

void ir_function_value_add(struct function *func, struct value *val)
{
        const char *lable = val->name;
        val->hash = fnv_1_hash(lable, strlen(lable));
        hash_add(&func->var_tbl, &val->hlist);
}

static void ir_function_destroy(struct function *func)
{
        /* free all entries in table: blocks and values */
        hash_free(&func->var_tbl);
        free(func);
}
