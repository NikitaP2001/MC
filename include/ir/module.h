#ifndef _IR_MODULE_H_
#define _IR_MODULE_H_
#include <ir/function.h>

struct ir_module {
        struct hash_table var_tbl;
        
        size_t num_functions;
        size_t num_globals;
};

struct ir_module *ir_module_create();

void ir_module_add_function(struct ir_module *module, struct function *func);

void ir_module_add_global(struct ir_module *module, struct ir_object *obj);

void ir_module_destroy(struct ir_module *module);

#endif /* _IR_MODULE_H_ */