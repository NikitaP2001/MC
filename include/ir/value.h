#ifndef _IR_VALUE_H_
#define _IR_VALUE_H_
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <tools/hashtable.h>
#include <ir/ins.h>
#include <mc.h>
#include <vector.h>
#include <fixed_str.h>

enum ir_value_type {
        ir_value_basic_block,
        ir_value_object,
        /* actual function when func ptr is object */
        ir_value_function,
        ir_value_void,
};

struct ir_value {
        struct fixed_str name;
        enum ir_value_type type;
        struct vector ins_use;

        hash_key_t hash;
        struct hlist_entry hlist;
};

static inline const char *ir_value_name_get(struct ir_value *val)
{
        return fixed_str_get(&val->name);
}

void ir_value_add_use(struct ir_value *val, struct ir_ins_use *use);

struct ir_value_params {
        const char *prefix;
        uint32_t index;
        struct token *id;
};

void ir_value_init(struct ir_value *val, struct ir_value_params *params);

static inline 
struct ir_value *
ir_value_hlist_entry(struct hlist_entry *node)
{
        return container_of(node, struct ir_value, hlist);
}

hash_key_t ir_value_node_hash(struct hlist_entry *node);

hash_key_t ir_value_hash(struct ir_value *value);

void ir_value_free(struct hlist_entry *node);

#endif /* _IR_VALUE_H_ */