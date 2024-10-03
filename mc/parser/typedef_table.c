#include <stdlib.h>
#include <assert.h>

#include <parser/typedef_table.h>
#include <tools/fnv_1.h>

static inline hash_key_t typedef_token_hash(struct token *id)
{
        union token_value *val = &id->value;
        return fnv_1_hash(val->var_raw.value, val->var_raw.length);
}

hash_key_t typedef_table_hash(struct hlist_entry *node)
{
        struct typedef_table_entry *entry = container_of(node, 
                struct typedef_table_entry, hlist);
        return entry->hash;
}

void typedef_table_free(struct hlist_entry *node)
{
        struct typedef_table_entry *entry = container_of(node, 
                struct typedef_table_entry, hlist);
        free(entry);
}

void typedef_table_add(struct hash_table *tbl, struct token *id, 
                       struct pt_node *scope)
{
        assert(id->type == tok_identifier);
        struct typedef_table_entry *entry 
                = calloc(1, sizeof(struct typedef_table_entry));
        entry->id_typedef = id;
        entry->hash = typedef_token_hash(id);
        entry->scope = scope;
        hash_add(tbl, &entry->hlist);
}

_Bool typedef_table_contains(struct hash_table *tbl, struct token *id, 
                             struct pt_node **path, uint16_t path_len)
{
        assert(id->type == tok_identifier);
        hash_key_t key = typedef_token_hash(id);
        HASH_FOREACH_ENTRY(tbl, key) {
                struct typedef_table_entry *t_e = container_of(entry, 
                        struct typedef_table_entry, hlist);
                if (t_e->hash == key && token_val_cmp(id, 
                        t_e->id_typedef) == 0) {
                        for (uint16_t i = 0; i < path_len; i++) {
                                if (path[i] == t_e->scope)
                                        return true;
                        }
                }
        }
        return false;
}