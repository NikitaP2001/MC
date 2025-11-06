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

static inline void ir_module_value_add(struct ir_module *module, 
                                       struct ir_value *val)
{
        hash_add(&module->var_tbl, &val->hlist);
}

struct ir_value *ir_module_value_get(struct ir_module *module, 
                                     struct token *id_tok)
{
        return ir_seek_var_in_table(&module->var_tbl, id_tok);
}

void ir_module_function_create(struct ir_module *module, struct pt_node *func_def)
{
        struct ir_function *func = ir_function_create(func_def);
        ir_module_value_add(module, &func->val);
        module->num_functions++;
}

void ir_module_object_add(struct ir_module *module, struct ir_object *obj)
{
        module->num_globals++;
        ir_module_value_add(module, &obj->val);
}
