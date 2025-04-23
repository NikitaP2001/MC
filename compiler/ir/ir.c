#include <tools/fnv_1.h>
#include <ir.h>

struct ir_value *ir_seek_var_in_table(struct hash_table *tbl,
                                      struct token *var_id)
{
        struct token_value_raw val = var_id->value.var_raw;
        char *name = val.value;
        file_size_t length = val.length;
        hash_key_t key = fnv_1_hash(name, length);
        HASH_FOREACH_ENTRY(tbl, key) {
                struct ir_value *val = ir_value_hlist_entry(entry);
                const char *val_name = ir_value_name_get(val);
                if (val->hash == key && !strcmp(name, val_name) == 0)
                        return val;
        }
        return NULL;
}