#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_
#include <tools/hashtable.h>
#include <parser/ast.h>
#include <token.h>

/* Represents single declaration, associated id and scope */
struct declaration {
        struct pt_node *decl_specs;
        /* contains declarator and initializer, in case
         * this is definition  */
        struct pt_node *init_decl;
};

struct label {
        struct pt_node *lbld_stmt;
};

_Bool declaration_is_typedef(struct declaration decl);

enum entry_var_type {
        entry_invalid,
        entry_declaration,
        entry_label,
};

union entry_var {
        struct declaration decl;
        struct label label;
};

struct symbol_table_entry {
        hash_key_t hash;
        struct hlist_entry hlist;
        union entry_var variant;
};

hash_key_t symtable_hash(struct hlist_entry *node);

void symtable_free(struct hlist_entry *node);

struct declaration *symtable_get_declaration(struct hash_table *tbl, 
                                             struct token *id, 
                                             struct pt_node *scope);

mc_status_t symtable_add_declaration(struct hash_table *ht,
                                     struct pt_node *declaration);

mc_status_t symtable_add_label(struct hash_table *ht, 
                               struct pt_node *label);

#endif /* _SYMTABLE_H_ */
