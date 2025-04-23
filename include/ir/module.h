#ifndef _IR_MODULE_H_
#define _IR_MODULE_H_
#include <ir/function.h>

struct module {
        struct hash_table var_tbl;
};

struct module *ir_module_create();

void ir_module_add_function(struct module *module, struct function *func);

void ir_module_destroy(struct module *module);

#endif /* _IR_MODULE_H_ */