#include <ir/module.h>
#include <ir.h>

struct ir_module *ir_module_create()
{
        struct ir_module *module = calloc(1, sizeof(struct ir_module));
        struct hash_table_ops t_ops = {
                .free = ir_value_free,
                .get_key = ir_value_node_hash,
        };
        hash_init(&module->var_tbl, t_ops);
        return module;
}

void ir_module_destroy(struct ir_module *module)
{
        hash_free(&module->var_tbl);
        free(module);
}

static inline void ir_module_add_value(struct ir_module *module, 
                                       struct ir_value *val)
{
        hash_add(&module->var_tbl, &val->hlist);
}

void ir_module_add_function(struct ir_module *module, struct function *func)
{
        module->num_functions++;
        ir_module_add_value(module, &func->val);
}

void ir_module_add_global(struct ir_module *module, struct ir_object *obj)
{
        module->num_globals++;
        ir_module_add_value(module, &obj->val);
}

