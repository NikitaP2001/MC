#include <parser/symtable.h>
#include <parser/symbol.h>
#include <parser/ast.h>
#include <tools/fnv_1.h>
#include "symset.h"

/* TODO: add flags to declaration struct  indicating present specifiers,
 * also make check to not to add ambiguous specifiers */
static inline _Bool declaration_spec_is_typedef(struct pt_node *decl_spec)
{
        AST_FOREACH_CHILD(decl_spec) {
                struct pt_node *node = (struct pt_node *)entry;
                if (node->sym == psym_storage_class_specifier) {
                        node = pt_node_child_first(node);
                        if (token_get_keyword(node->value) == keyw_typedef)
                                return true;
                }

        }
        return false;
}

_Bool declaration_is_typedef(struct declaration decl)
{
        return declaration_spec_is_typedef(decl.decl_specs);
}

static inline hash_key_t symtable_token_hash(struct token *id)
{
        union token_value *val = &id->value;
        return fnv_1_hash(val->var_raw.value, val->var_raw.length);
}

static inline 
struct symbol_table_entry *
symtable_hlist_entry(struct hlist_entry *node)
{
        return container_of(node, struct symbol_table_entry, hlist);
}

hash_key_t symtable_hash(struct hlist_entry *node)
{
        return symtable_hlist_entry(node)->hash;
}

void symtable_free(struct hlist_entry *node)
{
        struct symbol_table_entry *entry = container_of(node, 
                struct symbol_table_entry, hlist);
        free(entry);
}

/* from init declarator extract associated identifier */
static struct token*
symtable_init_decl_id(struct pt_node *init_decl) 
{
        struct pt_node *dir_decl = init_decl;
        struct pt_node *decl;

        do {
                decl = pt_node_child_first(dir_decl);
                dir_decl = pt_node_child_last(decl);
        } while (decl->sym != psym_identifier);

        return decl->value;
}

static inline struct token*
symtable_label_id(struct pt_node *label) 
{
        return pt_node_child_first(label)->value;
}

enum entry_var_type symtable_get_var_type(union entry_var var)
{
        struct pt_node *node = var.decl.decl_specs;
        switch (node->sym) {
                case psym_declaration_specifiers:
                        return entry_declaration;
                case psym_labeled_statement:
                        return entry_label;
                default:
                        MC_LOG(MC_CRIT, "Unexpected var node");
                        break;
        }
        return entry_invalid;
}

static inline _Bool symtable_entry_is_decl(struct symbol_table_entry *e)
{
        return (symtable_get_var_type(e->variant) == entry_declaration);
}

static struct token*
symtable_entry_var_id(union entry_var var)
{

        struct token *id;
        switch (symtable_get_var_type(var)) {
                case entry_declaration:
                {
                        struct declaration decl = var.decl; 
                        id = symtable_init_decl_id(decl.init_decl);
                        break;
                }
                case entry_label:
                {
                        struct label lbl = var.label; 
                        id = symtable_label_id(lbl.lbld_stmt);
                        break;
                }
                        
                default:
                        id = NULL;
                        break;
        }
        return id;     
}

static void symtable_add(struct hash_table *tbl, union entry_var var)
{
        struct token *id = symtable_entry_var_id(var);
        struct symbol_table_entry *entry 
                = calloc(1, sizeof(struct symbol_table_entry));
        entry->hash = symtable_token_hash(id);
        entry->variant = var;
        hash_add(tbl, &entry->hlist);
}

static _Bool symtable_decl_visible(struct declaration decl,
                                   struct pt_node *scope)
{
        static const uint8_t decl_sec[] = { DECLARATION_SECOND_PARENT };
        /* find actual scope declaration relates to */
        struct pt_node *node_decl = decl.decl_specs->parent;
        struct pt_node *decl_scope = node_decl->parent;
        if (decl_sec[decl_scope->sym])
                decl_scope = decl_scope->parent;
        /* go up the tree, looking if we will reach declaration scope */
        while (scope != NULL) {
                if (scope == decl_scope)
                        return true;
                scope = scope->parent;
        }
        return false;
}


struct declaration *symtable_get_declaration(struct hash_table *tbl, 
                                             struct token *id, 
                                             struct pt_node *scope)
{
        assert(id->type == tok_identifier);
        hash_key_t key = symtable_token_hash(id);
        HASH_FOREACH_ENTRY(tbl, key) {
                struct symbol_table_entry *t_e = symtable_hlist_entry(entry);
                /* variable/typedef shadowing - variables of our scope
                 * declared earlier - are higher in the hlist, so we
                 * will find out first most recent added one. We will
                 * pick it, and assume that other ones are shadowed */
                if (t_e->hash == key && symtable_entry_is_decl(t_e)) {
                        struct declaration *decl = &t_e->variant.decl;
                        struct token *id_decl = symtable_init_decl_id(
                                decl->init_decl);
                        if (token_compare(id, id_decl) == 0 
                                && symtable_decl_visible(*decl, scope))
                                return decl;
                }
        }
        return NULL;
}

mc_status_t symtable_add_declaration(struct hash_table *ht,
                                     struct pt_node *declaration)
{
        struct pt_node *init_lst = pt_node_child_last(declaration);
        struct pt_node *decl_spec = pt_node_child_first(declaration);

        assert(declaration->sym == psym_declaration);
        /* we will be unable to identify scope */
        assert(declaration->parent != NULL);
        /* init declarator list is optional*/
        if (init_lst->sym == psym_declaration_specifiers)
                return MC_OK;

        AST_FOREACH_CHILD(init_lst) {
                struct pt_node *init_decl = (struct pt_node *)entry;

                union entry_var var = {
                        .decl = {
                                .decl_specs = decl_spec,
                                .init_decl = init_decl,
                        },
                };
                /* TODO: 
                 * 1. check 6.7p2 no more than one declaration of the 
                 * identifier with the same scope and in the same name 
                 * space and other..
                 * 2. Add also enums and structs declarations from specifiers  */
                symtable_add(ht, var);
        }
        return MC_OK;
}

mc_status_t symtable_add_label(struct hash_table *ht,
                               struct pt_node *label)
{
        assert(label->sym == psym_labeled_statement);
        struct token *id_label = symtable_label_id(label);
        if (id_label->type != tok_identifier)
                return MC_FAIL;

        union entry_var var = {
                .label = {
                        .lbld_stmt = label,
                },
        };
        symtable_add(ht, var);
        return MC_OK;
}