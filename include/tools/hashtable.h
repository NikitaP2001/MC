#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdlib.h>
#include <mc/types.h>

#define HASH_SIZE_INIT 32
#define HASH_SIZE_THRESHOLD 0.75

struct hlist_entry {
        struct hlist_entry *next;
        /* we do not support delete now, but later we may need pprev
         * struct hlist_entry **pprev; */
};

typedef hash_key_t (*hash_get_t)(struct hlist_entry *);
typedef void (*free_entry_t)(struct hlist_entry *);

struct hash_table_ops {
        hash_get_t get_key;
        free_entry_t free;
};

struct hash_table {
        struct hlist_entry **first;
        /* amount of slots */
        size_t size;
        /* total emount of added elements */
        size_t nodes;
        struct hash_table_ops ops;
};

static inline void hlist_add_head(struct hlist_entry *node, 
                                  struct hlist_entry **pphead)
{
        struct hlist_entry *head = *pphead;
        node->next = head;
        *pphead = node;
}

static inline hash_key_t hash_get_key(struct hash_table *table, 
                                      struct hlist_entry *node)
{
        return table->ops.get_key(node);
}

#define HASH_TABLE_KEY(table, node) hash_get_key(table, node)

inline static void hash_init(struct hash_table *table, 
                   struct hash_table_ops ops)
{
        table->size = HASH_SIZE_INIT;
        table->first = calloc(table->size, sizeof(struct hlist_entry **));
        table->nodes = 0;
        table->ops = ops;
}

inline static void hash_add(struct hash_table *table, 
                            struct hlist_entry *node);

#define HASH_FOREACH_ENTRY(table, key)                                  \
        for (struct hlist_entry *entry = *__hash_hlist(table, key);     \
             entry != NULL;                                             \
             entry = entry->next)

inline static void __hash_rehash(struct hash_table *table)
{
        size_t old_size = table->size;
        struct hlist_entry **old_first = table->first;
        table->first = calloc(table->size, sizeof(struct hlist_entry **));
        table->size = old_size * 2;
        table->nodes = 0;

        for (size_t i_key = 0; i_key < old_size; i_key++) {
                struct hlist_entry *head = old_first[i_key];
                while (head != NULL) {
                        struct hlist_entry *next = head->next;
                        hash_add(table, head);
                        head = next;
                }
        }
        free(old_first);
}

inline static struct hlist_entry **__hash_hlist(struct hash_table *table,
                                                hash_key_t key)
{
        return &table->first[key % table->size];
}

inline static void hash_add(struct hash_table *table, 
                            struct hlist_entry *node)
{
        hash_key_t key = HASH_TABLE_KEY(table, node);
        if (table->nodes >= table->size * 0.75)
                __hash_rehash(table);
        hlist_add_head(node, __hash_hlist(table, key));
}

static inline void hash_free(struct hash_table *table)
{
        for (size_t i_key = 0; i_key < table->size; i_key++) {
                struct hlist_entry *head = table->first[i_key];
                while (head != NULL) {
                        struct hlist_entry *next = head->next;
                        table->ops.free(head);
                        head = next;
                }
        }
        free(table->first);
}

#endif /* _HASHTABLE_H_ */