#ifndef _FUNCTION_H_
#define _FUNCTION_H_
#include <ir/basic_block.h>
#include <tools/hashtable.h>
#include <parser/ast.h>

struct ir_function {
        _Bool is_static;
        /* replace usage with value
        struct token *name;
        */

        struct ir_value val; 

        struct hash_table var_tbl; 

        struct basic_block *entry;
};

struct ir_function *ir_function_create(struct pt_node *func_def);

void ir_function_destroy(struct ir_function *func);

void ir_function_value_add(struct ir_function *func, struct ir_value *val);

struct ir_value *ir_function_value_get(struct ir_function *func, 
                                       struct token *id_tok);

static inline 
struct ir_function *
ir_value_function_get(struct ir_value *val)
{
        assert(val->type == ir_value_function);
        return container_of(val, struct ir_function, val);
}

static inline 
void 
ir_function_block_add(struct ir_function *func, struct basic_block *bb)
{
        if (func->entry == NULL)
                func->entry = bb;
        ir_function_value_add(func, &bb->val);
}

#endif /* _FUNCTION_H_ */