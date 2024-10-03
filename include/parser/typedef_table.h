#ifndef _TYPEDEF_TABLE_H_
#define _TYPEDEF_TABLE_H_
#include <tools/hashtable.h>
#include <parser/tree.h>
#include <token.h>

struct typedef_table_entry {
        struct token *id_typedef;
        hash_key_t hash;
        struct hlist_entry hlist;
        struct pt_node *scope;
};

hash_key_t typedef_table_hash(struct hlist_entry *node);

void typedef_table_free(struct hlist_entry *node);

void typedef_table_add(struct hash_table *tbl, struct token *id, 
                       struct pt_node *scope);

_Bool typedef_table_contains(struct hash_table *tbl, struct token *id, 
                             struct pt_node **path, uint16_t path_len);

#endif /* _TYPEDEF_TABLE_H_ */