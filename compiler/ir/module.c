#include <ir/module.h>
#include <ir.h>

struct module *ir_module_create()
{
        struct module *module = calloc(1, sizeof(struct module));
        struct hash_table_ops t_ops = {
                .free = ir_value_free,
                .get_key = ir_value_node_hash,
        };
        hash_init(&module->var_tbl, t_ops);
        return module;
}

void ir_module_destroy(struct module *module)
{
        assert(false);
        hash_free(&module->var_tbl);
        free(module);
}

void ir_module_add_function(struct module *module, struct function *func)
{
        hash_add(&module->var_tbl, &func->val.hlist);
}

