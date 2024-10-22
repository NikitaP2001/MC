#ifndef _IR_H_
#define _IR_H_
#include <stdint.h>
#include <list.h>
#include <parser/ast.h>
#include <tools/hashtable.h>

struct module {
        // values set
        // functions set
        struct function **functions;
        size_t func_count;
};

struct module *ir_module_create();

void ir_module_destroy(struct module *module);

struct instruction {
        struct list_head *head;
};

struct value {
        char *name;

};

#define IR_BASIC_BLOCK_LABLE_MAX 32

struct basic_block {
        char lable[IR_BASIC_BLOCK_LABLE_MAX];

        struct instruction *first;
        size_t ins_count;

        hash_key_t hash;
        struct hlist_entry hlist;
};

struct basic_block *ir_bb_create(const char *prefix, uint32_t index);

struct function {
        _Bool is_static;
        struct token *name;
        /* table for basic blocks */
        struct hash_table tbl; 
        struct hash_table var_tbl; 

        struct basic_block *entry;
};

struct function *ir_function_create(struct module *module, 
                                    struct pt_node *func_def);

void ir_function_block_add(struct function *func, struct basic_block *bb);

#endif /* _IR_H_ */