#include <parser/symtable.h>
#include <parser/symbol.h>
#include <parser/ast.h>
#include <tools/fnv_1.h>
#include "symset.h"

typedef _Bool (*symtable_entry_visible_t)(struct symtable_entry *entry, struct pt_node *scope);
typedef mc_status_t (*symtable_entry_add_t)(struct symtable *sym_tbl, struct pt_node *label);
typedef void (*symtable_entry_free_t)(union entry_var *var);

struct symtable_entry_ops {
        symtable_entry_visible_t visible;
        symtable_entry_add_t     add;
        symtable_entry_free_t    free;
};

static _Bool symtable_declaration_visible(struct symtable_entry *entry, 
                                          struct pt_node *scope);
static _Bool symtable_label_visible(struct symtable_entry *entry, 
                                    struct pt_node *scope);
static _Bool symtable_type_visible(struct symtable_entry *entry, struct pt_node *scope);

mc_status_t symtable_add_declaration(struct symtable *sym_tbl, struct pt_node *decl);
mc_status_t symtable_add_label(struct symtable *sym_tbl, struct pt_node *label);

static void symtable_free_declaration(union entry_var *var);
static void symtable_free_type_info(union entry_var *var);

struct symtable_entry_ops entry_ops_table[] = {
        [entry_definition] = {
                .visible = symtable_declaration_visible,
                .add = symtable_add_declaration,
                .free = symtable_free_declaration,
        },
        [entry_label] = {
                .visible = symtable_label_visible,
                .add = symtable_add_label,
        },
        [entry_type] = {
                .visible = symtable_type_visible,
                .add = NULL,
                .free = symtable_free_type_info,
        },
};
#define ENTRY_OPS(entry) entry_ops_table[entry->var_type]

static inline 
struct symtable_entry *
symtable_hlist_entry(struct hlist_entry *node)
{
        return container_of(node, struct symtable_entry, hlist);
}

static void symtable_free_declaration(union entry_var *var)
{
        UNUSED(var);
        assert(false);
}

static void symtable_free_type_info(union entry_var *var)
{
        UNUSED(var);
        assert(false);
}

static void table_free(struct hlist_entry *node)
{
        struct symtable_entry *entry = symtable_hlist_entry(node);
        if (ENTRY_OPS(entry).free != NULL)
                ENTRY_OPS(entry).free(&entry->variant);
        free(entry);
}

_Bool declaration_is_typedef(struct declaration decl)
{
        struct declaration_specifiers *specs = &decl.specs;
        return (specs->storage_class & scs_typedef);
}

void declaration_set_scope(struct declaration *decl, struct pt_node *decl_node)
{
        static const uint8_t decl_sec[] = { DECLARATION_SECOND_PARENT };
        /* find actual scope declaration relates to */
        struct pt_node *decl_scope = decl_node->parent;
        if (decl_sec[decl_scope->sym])
                decl_scope = decl_scope->parent;
        decl->scope = decl_scope;
}

static inline hash_key_t symtable_token_hash(struct token *id)
{
        union token_value *val = &id->value;
        return fnv_1_hash(val->var_raw.value, val->var_raw.length);
}

static hash_key_t symtable_hash(struct hlist_entry *node)
{
        return symtable_hlist_entry(node)->hash;
}

void symtable_init(struct symtable *table)
{
        struct hash_table_ops hops = {
                .get_key = symtable_hash,
                .free = table_free,
        };
        hash_init(&table->id_tbl, hops);
        hash_init(&table->lbl_table, hops);
        hash_init(&table->type_tbl, hops);
}

void symtable_free(struct symtable *table)
{
        hash_free(&table->id_tbl);
        hash_free(&table->lbl_table);
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

static inline _Bool symtable_entry_is_decl(struct symtable_entry *e)
{
        return (e->var_type == entry_definition);
}

static inline _Bool symtable_entry_is_label(struct symtable_entry *e)
{
        return (e->var_type == entry_label);
}

static inline 
void symtable_insert_entry(struct hash_table *tbl, struct symtable_entry *entry)
{
        entry->hash = symtable_token_hash(entry->id);
        hash_add(tbl, &entry->hlist);
}

static _Bool symtable_type_visible(struct symtable_entry *entry, struct pt_node *scope)
{
        UNUSED(entry);
        UNUSED(scope);
        assert(false);
        return false;
}

mc_status_t symtable_add_declaration(struct symtable *sym_tbl, struct pt_node *decl_node)
{
        mc_status_t status = MC_OK;
        assert(decl_node->sym == psym_declaration);
        struct pt_node *init_lst = pt_node_child_last(decl_node);
        struct declaration decl = {0};

        declaration_set_scope(&decl, decl_node);
        status = type_set_declaration_info(sym_tbl, &decl);
        if (!MC_SUCC(status))
                return status;

        /* save type to sym_tbl symtable_insert_type */

        if (init_lst->sym != psym_init_declarator_list)
                return MC_OK;

        /* we will be unable to identify scope */
        assert(decl_node->parent != NULL);

        AST_FOREACH_CHILD(init_lst) {
                struct pt_node *init_decl = (struct pt_node *)entry;

                struct symtable_entry *entry
                        = calloc(1, sizeof(struct symtable_entry));
                entry->var_type = entry_definition;
                entry->id = symtable_init_decl_id(init_decl);
                entry->variant = (union entry_var){ .decl = decl };
                /* TODO: 
                 * 1. check 6.7p2 no more than one declaration of the 
                 * identifier with the same scope and in the same name 
                 * space and other. */
                symtable_insert_entry(&sym_tbl->id_tbl, entry);
        }

        return MC_OK;
}

static _Bool symtable_declaration_visible(struct symtable_entry *entry,
                                          struct pt_node *scope)
{
        struct declaration *decl = &entry->variant.decl;
        struct pt_node *decl_scope = decl->scope;

        /* go up the tree, looking if we will reach declaration scope */
        while (scope != NULL) {
                if (scope == decl_scope)
                        return true;
                scope = scope->parent;
        }
        return false;
}

static void label_set_scope(struct label *lbl, struct pt_node *label_node)
{
        struct pt_node *lbl_scope = label_node->parent;
        while (lbl_scope->sym != psym_function_definition && lbl_scope != NULL)
                lbl_scope = lbl_scope->parent;       
        /* label have no function def in parent */
        assert(lbl_scope != NULL);
        lbl->scope = lbl_scope;
}

mc_status_t symtable_add_label(struct symtable *sym_tbl, struct pt_node *label)
{       
        struct hash_table *ht = &sym_tbl->lbl_table;
        assert(label->sym == psym_labeled_statement);
        struct token *id_label = symtable_label_id(label);
        if (id_label->type != tok_identifier)
                return MC_FAIL;
        struct label lbl = {0};
        label_set_scope(&lbl, label);

        struct symtable_entry *entry
                = calloc(1, sizeof(struct symtable_entry));
        entry->var_type = entry_label;
        entry->id = id_label;
        entry->variant = (union entry_var){ .label = lbl };

        symtable_insert_entry(ht, entry);
        return MC_OK;
}

static _Bool symtable_label_visible(struct symtable_entry *entry,
                                    struct pt_node *scope)
{
        struct label lbl = entry->variant.label;
        struct pt_node *lbl_scope = lbl.scope;
        /* go up the tree, looking if we will reach label scope */
        while (scope != NULL) {
                if (scope == lbl_scope)
                        return true;
                scope = scope->parent;
        }
        return false;
}


struct symtable_entry *symtable_get(struct hash_table *tbl, 
                                    struct token *id, 
                                    struct pt_node *scope)
{
        assert(id->type == tok_identifier);
        hash_key_t key = symtable_token_hash(id);
        HASH_FOREACH_ENTRY(tbl, key) {
                struct symtable_entry *t_e = symtable_hlist_entry(entry);
                /* variable shadowing - variables of our scope
                 * declared earlier - are higher in the hlist, so we
                 * will find out first most recent added one. We will
                 * pick it, and assume that other ones are shadowed */
                if (t_e->hash == key) {
                        struct token *id_entry = t_e->id;
                        if (token_compare(id, id_entry) == 0 
                                && ENTRY_OPS(t_e).visible(t_e, scope))
                                return t_e;
                }
        }
        return NULL;
}