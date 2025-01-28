#ifndef _IR_H_
#define _IR_H_
#include <list.h>
#include <ir/value.h>
#include <ir/ins.h>
#include <ir/gen.h>
#include <parser/ast.h>

struct module {
        // values set
        // functions set
        struct function **functions;
        size_t func_count;
};

struct module *ir_module_create();

void ir_module_destroy(struct module *module);

struct function {
        _Bool is_static;
        struct token *name;

        struct hash_table var_tbl; 

        struct basic_block *entry;
};

struct function *ir_function_create(struct module *module, 
                                    struct pt_node *func_def);

void ir_function_value_add(struct function *func, struct value *val);

static inline 
void 
ir_function_block_add(struct function *func, struct basic_block *bb)
{
        if (func->entry == NULL)
                func->entry = bb;
        ir_function_value_add(func, &bb->val);
}

#endif /* _IR_H_ */