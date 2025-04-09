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
                        if (token_get_keyword(node->node_value.value) 
                                == keyw_typedef)
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

static hash_key_t symtable_hash(struct hlist_entry *node)
{
        return symtable_hlist_entry(node)->hash;
}

static void table_free(struct hlist_entry *node)
{
        free(symtable_hlist_entry(node));
}

void symtable_init(struct symtable *table)
{
        struct hash_table_ops hops = {
                .get_key = symtable_hash,
                .free = table_free,
        };
        hash_init(&table->id_tbl, hops);
        hash_init(&table->type_tbl, hops);
}

void symtable_free(struct symtable *table)
{
        hash_free(&table->id_tbl);
        hash_free(&table->type_tbl);
}

/* from init declarator extract associated identifier */
static inline struct token*
symtable_init_decl_id(struct pt_node *init_decl) 
{
        struct pt_node *decl = pt_node_child_first(init_decl);
        return ast_declarator_id(decl);
}

static inline struct token*
symtable_label_id(struct pt_node *label) 
{
        return pt_node_child_first(label)->node_value.value;
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
                        MC_DBG(MC_CRIT, "Unexpected var node");
                        break;
        }
        return entry_invalid;
}

static inline _Bool symtable_entry_is_decl(struct symbol_table_entry *e)
{
        return (symtable_get_var_type(e->variant) == entry_declaration);
}

static inline _Bool symtable_entry_is_lable(struct symbol_table_entry *e)
{
        return (symtable_get_var_type(e->variant) == entry_label);
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

static _Bool symtable_label_visible(struct label lbl,
                                    struct pt_node *scope)
{
        /* find actual scope declaration relates to */
        struct pt_node *lbl_scope = lbl.lbld_stmt->parent;
        while (lbl_scope->sym != psym_function_definition && lbl_scope != NULL)
                lbl_scope = lbl_scope->parent;
        if (lbl_scope == NULL) {
                MC_DBG(MC_ERR, "label have no function def in parent");
                return false;
        }

        /* go up the tree, looking if we will reach label scope */
        while (scope != NULL) {
                if (scope == lbl_scope)
                        return true;
                scope = scope->parent;
        }
        return false;
}


struct declaration *symtable_get_declaration(struct symtable *sym_tbl,
                                             struct token *id, 
                                             struct pt_node *scope)
{
        struct hash_table *tbl = &sym_tbl->id_tbl;
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

struct label *symtable_get_label(struct symtable *sym_tbl,
                                 struct token *id, 
                                 struct pt_node *scope)
{
        struct hash_table *tbl = &sym_tbl->id_tbl;
        assert(id->type == tok_identifier);
        hash_key_t key = symtable_token_hash(id);
        HASH_FOREACH_ENTRY(tbl, key) {
                struct symbol_table_entry *t_e = symtable_hlist_entry(entry);
                /* variable/typedef shadowing - variables of our scope
                 * declared earlier - are higher in the hlist, so we
                 * will find out first most recent added one. We will
                 * pick it, and assume that other ones are shadowed */
                if (t_e->hash == key && symtable_entry_is_lable(t_e)) {
                        struct label *lbl = &t_e->variant.label;
                        struct token *id_lbl 
                                = symtable_label_id(lbl->lbld_stmt);
                        if (token_compare(id, id_lbl) == 0 
                                && symtable_label_visible(*lbl, scope))
                                return lbl;
                }
        }
        return NULL;
}

static mc_status_t symtable_add_declaration_type(struct hash_table *ht,
                                                 struct pt_node *decl_spec)
{
        UNUSED(ht);
        UNUSED(decl_spec);
        mc_status_t status = MC_OK;
        /*
        uint16_t child_count = pt_node_child_count(decl_spec);

        for (uint16_t i_node = 1; i_node <= child_count; i_node++) {
                struct pt_node *curr = pt_node_child_number(decl_spec, i_node);
                if (curr->sym == psym_storage_class_specifier) {

                }
        }
        assert(false);
        */
        return status;
}

mc_status_t symtable_add_declaration(struct symtable *sym_tbl,
                                     struct pt_node *decl)
{
        mc_status_t status = MC_OK;
        assert(decl->sym == psym_declaration);
        struct pt_node *init_lst = pt_node_child_last(decl);
        struct pt_node *decl_spec = pt_node_child_first(decl);

        status = symtable_add_declaration_type(&sym_tbl->type_tbl, decl);
        if (status != MC_OK)
                return status;

        if (init_lst->sym == psym_declaration_specifiers)
                return MC_OK;

        /* we will be unable to identify scope */
        assert(decl->parent != NULL);

        AST_FOREACH_CHILD(init_lst) {
                struct pt_node *init_decl = (struct pt_node *)entry;
                /* TODO: decl spec node should be replace with 
                 * type info parsed in add_decl_type */
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
                 * 2. Add also enums and structs declarations from specifiers */
                symtable_add(&sym_tbl->id_tbl, var);
        }

        return MC_OK;
}

mc_status_t symtable_add_label(struct symtable *sym_tbl,
                               struct pt_node *label)
{       
        struct hash_table *ht = &sym_tbl->id_tbl;
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