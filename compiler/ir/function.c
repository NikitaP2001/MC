#include <stdlib.h>
#include <tools/fnv_1.h>
#include <ir/module.h>
#include <ir/function.h>
#include <ir.h>
#include <parser/ast.h>

struct function *ir_function_create(struct module *module, 
                                    struct pt_node *func_def)
{
        struct function *func = calloc(1, sizeof(struct function));
        ir_module_add_function(module, func);

        struct pt_node *declarator = pt_node_child_number(func_def, 2);
        struct token *func_id = ast_declarator_id(declarator);

        struct ir_value_params params = {
                .id = func_id,
        };
        ir_value_init(&func->val, &params);
        func->module = module;
        struct hash_table_ops t_ops = {
                .free = ir_value_free,
                .get_key = ir_value_node_hash,
        };
        hash_init(&func->var_tbl, t_ops);
        return func;
}

void ir_function_value_add(struct function *func, struct ir_value *val)
{
        val->hash = ir_value_hash(val);
        hash_add(&func->var_tbl, &val->hlist);
}

struct ir_value *ir_function_value_seek(struct function *func, 
                                        struct pt_node *id)
{
        struct ir_value *result;
        struct token *token = ast_declarator_id(id);
        struct module *mod = func->module;

        result = ir_seek_var_in_table(&func->var_tbl, token);
        if (result == NULL) {
                /* seek variable in global table */
                result = ir_seek_var_in_table(&mod->var_tbl, token);
                if (result == NULL) {
                        /* search for function in global context */
                }
        }

        return result;
}

void ir_function_destroy(struct function *func)
{
        /* free all entries in table: blocks and values */
        hash_free(&func->var_tbl);
        free(func);
}
