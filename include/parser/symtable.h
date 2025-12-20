#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_
#include <tools/hashtable.h>
#include <parser/ast.h>
#include <token.h>

#define SCS_SYM_OFFSET(sym) (sym - keyw_storage_class_specifier_first)
#define SCS_KEYWORD_TO_ENUM(kw) (1 << SCS_SYM_OFFSET(kw))

enum storage_class_specifier {
        scs_auto = 1 << SCS_SYM_OFFSET(keyw_auto),
        scs_register = 1 << SCS_SYM_OFFSET(keyw_register),
        scs_static = 1 << SCS_SYM_OFFSET(keyw_static),
        scs_extern = 1 << SCS_SYM_OFFSET(keyw_extern),
        scs_typedef = 1 << SCS_SYM_OFFSET(keyw_typedef),
};

#define TS_SYM_OFFSET(sym) (sym - keyw_type_specifier_first)
#define TS_KEYWORD_TO_ENUM(kw) (1 << TS_SYM_OFFSET(kw))

enum type_specifier {
        ts_void = 1 << TS_SYM_OFFSET(keyw_void),
        ts_char = 1 << TS_SYM_OFFSET(keyw_char),
        ts_short = 1 << TS_SYM_OFFSET(keyw_short),
        ts_int = 1 << TS_SYM_OFFSET(keyw_int),
        ts_long = 1 << TS_SYM_OFFSET(keyw_long),
        ts_float = 1 << TS_SYM_OFFSET(keyw_float),
        ts_double = 1 << TS_SYM_OFFSET(keyw_double),
        ts_signed = 1 << TS_SYM_OFFSET(keyw_signed),
        ts_unsigned = 1 << TS_SYM_OFFSET(keyw_unsigned),
        ts_Bool = 1 << TS_SYM_OFFSET(keyw_Bool),
        ts_Complex = 1 << TS_SYM_OFFSET(keyw_Complex),
        ts_Imaginary = 1 << TS_SYM_OFFSET(keyw_Imaginary),
        ts_struct_or_union = ts_Imaginary << 1,
        ts_enum = ts_struct_or_union << 1,
        ts_typedef_name = ts_enum << 1,
};

#define TQ_SYM_OFFSET(sym) (sym - keyw_type_qualifier_first)
#define TQ_KEYWORD_TO_ENUM(kw) (1 << TQ_SYM_OFFSET(kw))

enum type_qualifier {
        tq_const = 1 << TQ_SYM_OFFSET(keyw_const),
        tq_restrict = 1 << TQ_SYM_OFFSET(keyw_restrict),
        tq_volatile = 1 << TQ_SYM_OFFSET(keyw_volatile),
};

enum function_specifier {
        fs_inline = 1,
};

struct declaration_specifiers {
        enum storage_class_specifier storage_class;
        enum type_specifier type_spec;
        enum type_qualifier type_qual;
        enum function_specifier function_spec;
};

struct type_info {
        struct declaration_specifiers base;
        enum { 
                TYPE_DIRECT,
                TYPE_POINTER,
                TYPE_ARRAY,
                TYPE_FUNCTION,
        } kind;
        struct type_info *target;
        size_t array_size;
        /* struct param_list *params */
        struct pt_node *func_params;
};

/* Represents single declaration, associated id and scope */
struct declaration {
        struct pt_node *decl_specs;
        struct pt_node *init_decl;
        struct type_info *type;
        /* contains declarator and initializer, in case
         * this is definition  
        struct declarator decl;
         */
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
