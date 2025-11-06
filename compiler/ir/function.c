#include <stdlib.h>
#include <tools/fnv_1.h>
#include <ir/function.h>
#include <ir.h>
#include <parser/ast.h>

struct ir_function *ir_function_create(struct pt_node *func_def)
{
        struct ir_function *func = calloc(1, sizeof(struct ir_function));

        struct pt_node *declarator = pt_node_child_number(func_def, 2);
        struct token *func_id = ast_declarator_id(declarator);

        struct ir_value_params params = {
                .id = func_id,
                .type = ir_value_function,
        };
        ir_value_init(&func->val, &params);
        struct hash_table_ops t_ops = {
                .free = ir_value_free,
                .get_key = ir_value_node_hash,
        };
        hash_init(&func->var_tbl, t_ops);
        return func;
}

void ir_function_value_add(struct ir_function *func, struct ir_value *val)
{
        val->hash = ir_value_hash(val);
        hash_add(&func->var_tbl, &val->hlist);
}

struct ir_value *ir_function_value_get(struct ir_function *func, 
                                       struct token *id_tok)
{
        return ir_seek_var_in_table(&func->var_tbl, id_tok);
}

void ir_function_destroy(struct ir_function *func)
{
        /* free all entries in table: blocks and values */
        hash_free(&func->var_tbl);
        free(func);
}
