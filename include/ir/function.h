#ifndef _FUNCTION_H_
#define _FUNCTION_H_
#include <ir/basic_block.h>
#include <tools/hashtable.h>
#include <parser/ast.h>

struct function {
        /* associated module */
        struct ir_module *module;

        _Bool is_static;
        /* replace usage with value
        struct token *name;
        */

        struct ir_value val; 

        struct hash_table var_tbl; 

        struct basic_block *entry;
};

struct function *ir_function_create(struct ir_module *module, 
                                    struct pt_node *func_def);

void ir_function_destroy(struct function *func);

void ir_function_value_add(struct function *func, struct ir_value *val);

static inline 
struct function *
ir_value_function_get(struct ir_value *val)
{
        assert(val->type == ir_value_function);
        return container_of(val, struct function, val);
}

static inline 
void 
ir_function_block_add(struct function *func, struct basic_block *bb)
{
        if (func->entry == NULL)
                func->entry = bb;
        ir_function_value_add(func, &bb->val);
}

struct ir_value *ir_function_value_seek(struct function *func,
                                        struct pt_node *id);


#endif /* _FUNCTION_H_ */