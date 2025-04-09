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
        /* associated basic block, which may be assigned later */
        void *block;
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

struct symtable {
        struct hash_table id_tbl;
        struct hash_table type_tbl;
};

void symtable_init(struct symtable *table);

void symtable_free(struct symtable *table);

struct declaration *symtable_get_declaration(struct symtable *sym_tbl,
                                             struct token *id,
                                             struct pt_node *scope);

struct label *symtable_get_label(struct symtable *sym_tbl,
                                 struct token *id, 
                                 struct pt_node *scope);

mc_status_t symtable_add_declaration(struct symtable *sym_tbl,
                                     struct pt_node *decl);

mc_status_t symtable_add_label(struct symtable *sym_tbl,
                               struct pt_node *label);


#endif /* _SYMTABLE_H_ */
