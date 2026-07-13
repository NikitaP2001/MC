#ifndef _TYPE_H_
#define _TYPE_H_
#include <token.h>

#define SCS_SYM_OFFSET(sym) (sym - keyw_storage_class_specifier_first)
#define SCS_KEYWORD_TO_ENUM(kw) (1 << SCS_SYM_OFFSET(kw))

enum storage_class_specifier {
        scs_none = 0,
        scs_auto = 1 << SCS_SYM_OFFSET(keyw_auto),
        scs_register = 1 << SCS_SYM_OFFSET(keyw_register),
        scs_static = 1 << SCS_SYM_OFFSET(keyw_static),
        scs_extern = 1 << SCS_SYM_OFFSET(keyw_extern),
        scs_typedef = 1 << SCS_SYM_OFFSET(keyw_typedef),
};

#define TS_SYM_OFFSET(sym) (sym - keyw_type_specifier_first)
#define TS_KEYWORD_TO_ENUM(kw) (1 << TS_SYM_OFFSET(kw))

enum type_specifier {
        ts_none = 0,
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
        ts_long_long = ts_Imaginary << 1,
        ts_struct_or_union = ts_long_long << 1,
        ts_enum = ts_struct_or_union << 1,
        ts_typedef_name = ts_enum << 1,
};

#define TQ_SYM_OFFSET(sym) (sym - keyw_type_qualifier_first)
#define TQ_KEYWORD_TO_ENUM(kw) (1 << TQ_SYM_OFFSET(kw))

enum type_qualifier {
        tq_none = 0,
        tq_const = 1 << TQ_SYM_OFFSET(keyw_const),
        tq_restrict = 1 << TQ_SYM_OFFSET(keyw_restrict),
        tq_volatile = 1 << TQ_SYM_OFFSET(keyw_volatile),
};

enum function_specifier {
        fs_none = 0,
        fs_inline = 1,
};

struct declaration_specifiers {
        enum storage_class_specifier storage_class;
        enum type_specifier type_spec;
        enum type_qualifier type_qual;
        enum function_specifier function_spec;
};

struct type_info_struct {
        uint32_t field_count;
        struct struct_field {
                enum type_specifier type_spec;
                enum type_qualifier type_qual;
        } fields[];
};

struct type_info_enum {
        uint32_t field_count;
        struct enum_const {
                struct token *id;
                int value;
        } consts[];
};

struct type_info_union {
        uint32_t field_count;
        struct union_field {
                enum type_specifier type_spec;
                enum type_qualifier type_qual;
        } fields[];
};

struct type_info {
        enum { 
                TYPE_INVALID = 0,
                TYPE_STRUCT,
                TYPE_UNION,
                TYPE_ENUM,
        } kind;
        union {
                struct type_info_struct *ti_struct;
                struct type_info_union *ti_union;
                struct type_info_enum *ti_enum;
        } info;
};

struct declaration;
struct symtable;

mc_status_t type_set_declaration_info(_IN struct symtable *sym_tbl, 
                                      _OUT struct declaration *decl);

#endif /* _TYPE_H_ */