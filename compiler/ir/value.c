#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <token.h>
#include <ir/basic_block.h>
#include <ir/function.h>
#include <ir/value.h>
#include <ir/ins.h>
#include <tools/fnv_1.h>

#define IR_VALUE_USE_COUNT 5

void ir_value_add_use(struct ir_value *val, struct ir_ins_use *use)
{
        vector_push_back(&val->ins_use, &use);
}

hash_key_t ir_value_node_hash(struct hlist_entry *node)
{
        return ir_value_hlist_entry(node)->hash;
}

hash_key_t ir_value_hash(struct ir_value *value)
{
        const char *name = ir_value_name_get(value);
        return IR_VALUE_NAME_HASH(name, strlen(name));
}

void ir_value_free(struct hlist_entry *node)
{
        struct ir_value *val = ir_value_hlist_entry(node);
        fixed_str_free(&val->name);
        vector_free(&val->ins_use);
        switch (val->type) {
                case ir_value_basic_block:
                        ir_bb_destroy(ir_value_bb_get(val));
                        break;
                case ir_value_object:
                        ir_obj_destroy(ir_value_object_get(val));
                        break;
                case ir_value_function:
                        ir_function_destroy(ir_value_function_get(val));
                        break;
                default:
                        MC_DBG(MC_ERR, "unexpected value type");
        }
}

static void ir_value_name_temp(struct ir_value *val, 
                               const char *prefix, 
                               uint32_t index)
{
        char name[FIXED_STR_SIZE];
        int len = snprintf(name, FIXED_STR_SIZE, "%s%d", prefix, index);
        if (len < 0) {
                MC_DBG(MC_ERR, "form name failed");
                return;
        };
        fixed_str_init(&val->name, name, len);
}

static void ir_value_name_fixed(struct ir_value *val, 
                                struct token *token)
{
        if (token->type != tok_identifier)
                abort();
        fixed_str_init(&val->name, token_get_strlit(token), 
                token_get_id_length(token));
}

void ir_value_init(struct ir_value *val, struct ir_value_params *params)
{
        struct token *token = params->id;
        if (token)
                ir_value_name_fixed(val, token);
        else
                ir_value_name_temp(val, params->prefix, params->index);
        val->hash = ir_value_hash(val);
        val->type = params->type;
        vector_init(&val->ins_use, IR_VALUE_USE_COUNT, 
                sizeof(struct ir_ins_use *));
}
