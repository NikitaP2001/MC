#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_
#include <tools/hashtable.h>
#include <parser/ast.h>
#include <type.h>

struct definition {
        /* may be used for function definition, or variable definition */
        struct pt_node *def_node;
};

/* Represents single declaration, associated id and scope */
struct declaration {
        struct declaration_specifiers specs;
        struct pt_node *scope;
        struct type_info type;
        struct definition def;
};

struct label {
        struct pt_node *scope;
        /* associated basic block, which may be assigned later */
        void *block;
};

_Bool declaration_is_typedef(struct declaration decl);

void declaration_set_scope(struct declaration *decl, 
                           struct pt_node *decl_node);


enum entry_var_type {
        entry_invalid,
        entry_definition,
        entry_type,
        entry_label,
};

struct symtable_entry {
        hash_key_t hash;
        struct hlist_entry hlist;
        struct token *id;
        enum entry_var_type var_type;
        union entry_var {
                struct declaration decl;
                struct label label;
                struct type_info type;
        } variant;
};

struct symtable {
        struct hash_table id_tbl;
        struct hash_table lbl_table;
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

static inline void symtable_parse_error(struct pt_node *node, const char *message)
{
        if (node !=  NULL) {
                printf("error: %s", message);
                token_print(pt_node_value_get(node));
        }
}

#define SYMTABLE_ERROR(node, level, msg)                \
        MC_DBG(level, msg);                             \
        symtable_parse_error(node, msg)

#endif /* _SYMTABLE_H_ */
