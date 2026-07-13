#include <stdlib.h>
#include <parser/symbol.h>
#include <parser/symtable.h>
#include <type.h>

static 
enum type_specifier  
type_fetch_type_specifier(struct pt_node *ts_node);

static 
enum type_qualifier 
type_fetch_type_qualifier(struct pt_node *tq_node);

static 
enum storage_class_specifier 
type_fetch_storage_class_specifier(struct pt_node *scs_node)
{
        enum storage_class_specifier scs = scs_none;
        struct pt_node *val_node = pt_node_child_first(scs_node);
        enum keyword_type keyw_type 
                = token_get_keyword(pt_node_value_get(val_node));
        scs = SCS_KEYWORD_TO_ENUM(keyw_type); 
        return scs;
}

static void type_set_struct_type_spec(struct type_info *t_info,
                                      struct pt_node *spec_list_node)
{
        UNUSED(t_info);
        for (uint16_t i_node = 1; 
                i_node <= pt_node_child_count(spec_list_node); 
                i_node++) {
                struct pt_node *spec_node 
                        = pt_node_child_number(spec_list_node, i_node);
                switch (spec_node->sym) {
                case psym_type_specifier:
                        type_fetch_type_specifier(spec_node);
                        break;
                case psym_type_qualifier:
                        type_fetch_type_qualifier(spec_node);
                        break;
                default:
                        SYMTABLE_ERROR(spec_node, MC_ERR, 
                                "Unexpected struct decl specifier");
                        break;
                }
        }
}

static mc_status_t type_set_struct_decl_list(struct type_info *t_info,
                                            struct pt_node *decl_list_node)
{
        int field_count = pt_node_child_count(decl_list_node);

        if (field_count == 0) {
                SYMTABLE_ERROR(decl_list_node, MC_ERR, 
                        "Empty struct/union declaration list");
                return MC_FAIL;
        }
        t_info->info.ti_struct = calloc(1, 
                sizeof(struct type_info_struct) 
                + field_count * sizeof(struct struct_field));
        for (uint16_t i_node = 1; i_node <= field_count; i_node++) {
                struct pt_node *decl_node 
                        = pt_node_child_number(decl_list_node, i_node);
                struct pt_node *spec_list_node 
                        = pt_node_child_first(decl_node);
                /*
                struct pt_node *struc_decl_list 
                        = pt_node_child_last(decl_node);
                */
                
                type_set_struct_type_spec(t_info, spec_list_node);

        }
        return MC_OK;
}

static mc_status_t type_apply_struct_or_union_specifier(struct type_info *t_info,
                                                     struct pt_node *su_node)
{
        struct pt_node *node = pt_node_child_first(su_node);

        struct pt_node *st_or_u = pt_node_child_first(node);
        struct token *tag_tok = pt_node_value_get(st_or_u);
        enum keyword_type keyw_type = token_get_keyword(tag_tok);

        struct token* id_tok = NULL;
        uint16_t i_node = 2;
        struct pt_node *node = pt_node_child_number(su_node, i_node);

        if (node->sym == psym_identifier) {
                struct token *id_tok = pt_node_value_get(node);
                i_node++;
        }

        struct pt_node *decl_list_node = pt_node_child_number(su_node, i_node);
        status = type_set_struct_decl_list(t_info, decl_list_node);
        if (!MC_SUCC(status))
                return status;
        
        symtable_insert_type(&sym_tbl->type_tbl, t_info);
        return MC_OK;
}

static void type_apply_enum_specifier(struct type_info *t_info,
                                        struct pt_node *en_node)
{

}

static void type_apply_typedef_name(struct type_info *t_info,
                                    struct pt_node *tdef_node)
{

}

static 
enum type_qualifier 
type_fetch_type_qualifier(struct pt_node *tq_node)
{
        enum type_qualifier tq = tq_none;
        struct pt_node *val_node = pt_node_child_first(tq_node);
        enum keyword_type keyw_type 
                = token_get_keyword(pt_node_value_get(val_node));
        tq = TQ_KEYWORD_TO_ENUM(keyw_type);
        return tq;
}

static 
enum function_specifier 
type_fetch_function_specifier(struct pt_node *su_node)
{
        enum function_specifier fs = fs_none;
        struct pt_node *val_node = pt_node_child_first(su_node);
        enum keyword_type keyw_type 
                = token_get_keyword(pt_node_value_get(val_node));
        if (keyw_type == keyw_inline) {
                fs = fs_inline;
        } else {
                MC_DBG(MC_CRIT, "Unexpected function specifier");
        }
        return fs;
}

static 
enum type_specifier  
type_fetch_type_specifier(struct pt_node *ts_node)
{
        enum type_specifier t_spec = ts_none;
        struct pt_node *val_node = pt_node_child_first(ts_node);
        if (val_node->sym == psym_struct_or_union_specifier) {
                t_spec = ts_struct_or_union;
        } else if (val_node->sym == psym_enum_specifier) {
                t_spec = ts_enum;
        } else if (val_node->sym == psym_typedef_name) {
                t_spec = ts_typedef_name;
        } else {
                enum keyword_type keyw_type 
                        = token_get_keyword(pt_node_value_get(val_node));
                t_spec = TS_KEYWORD_TO_ENUM(keyw_type); 
        }
        return t_spec;
}

static mc_status_t type_verify_type_specifier(enum type_specifier spec)
{
        static int allowed_combinations[] = {
                ts_void,
                ts_char,
                ts_signed | ts_char,
                ts_unsigned | ts_char,
                ts_short, 
                ts_signed | ts_short,
                ts_short | ts_int,
                ts_signed | ts_short | ts_int,
                ts_unsigned | ts_short,
                ts_unsigned | ts_short | ts_int,
                ts_int, ts_signed, ts_signed | ts_int,
                ts_unsigned, ts_unsigned | ts_int,
                ts_long, ts_signed | ts_long,
                ts_long | ts_int, ts_signed | ts_long | ts_int,
                ts_unsigned | ts_long,
                ts_unsigned | ts_long | ts_int,
                ts_long_long, ts_signed | ts_long_long,
                ts_long_long | ts_int,
                ts_signed | ts_long_long | ts_int,
                ts_unsigned | ts_long_long,
                ts_unsigned | ts_long_long | ts_int,
                ts_float,
                ts_double,
                ts_long | ts_double,
                ts_Bool,
                ts_float | ts_Complex,
                ts_double | ts_Imaginary,
                ts_long | ts_double | ts_Complex,
                ts_float | ts_Imaginary,
                ts_double | ts_Imaginary,
                ts_long | ts_double | ts_Imaginary,
                ts_struct_or_union,
                ts_enum,
                ts_typedef_name,
        };
        for (size_t i = 0; i < ARRAY_SIZE(allowed_combinations); i++) {
                if (spec == allowed_combinations[i])
                        return MC_OK;
        }
        return MC_INVALID_TYPE;
        
}

static mc_status_t type_set_type_specifier(struct type_info *t_info, 
                                                struct pt_node *curr,
                                                enum type_specifier t_spec)
{
        mc_status_t status;
        enum type_specifier info_spec = t_info->base.type_spec;
        
        if (info_spec & t_spec) {
                if (t_spec == ts_long) {
                        info_spec ^= t_spec;
                        info_spec |= ts_long_long;
                } else {
                        SYMTABLE_ERROR(curr, MC_ERR, "Duplicate type specifier");
                        return MC_PARSE_ERROR;
                }
        } else {
                info_spec |= t_spec;
        }

        status = type_verify_type_specifier(info_spec);
        if (!MC_SUCC(status)) {
                SYMTABLE_ERROR(curr, MC_ERR, 
                        "Invalid type specifier combination");
                return status;
        }
        specs_info->type_spec = info_spec;
        if (!(info_spec & (ts_struct_or_union | ts_enum | ts_typedef_name)))
                return MC_OK;
        struct pt_node *val_node = pt_node_child_first(curr);
        if (info_spec & ts_struct_or_union) {
                status = type_apply_struct_or_union_specifier(t_info, val_node);
        } else if (info_spec & ts_enum) {
                type_apply_enum_specifier(t_info, val_node);
        } else if (info_spec & ts_typedef_name) {
                type_apply_typedef_name(t_info, val_node);
        }

        return status;
}

mc_status_t type_set_declaration_info(_IN struct symtable *sym_tbl, 
                                      _OUT struct declaration *decl)
{
        mc_status_t status = MC_OK;
        struct declaration_specifiers *specs_info = &decl->specs;
        struct type_info *t_info = &decl->type;
        struct pt_node *decl_spec = pt_node_child_first(decl);
        uint16_t child_count = pt_node_child_count(decl_spec);

        for (uint16_t i_node = 1; i_node <= child_count; i_node++) {
                struct pt_node *curr = pt_node_child_number(decl_spec, i_node);
                switch (curr->sym) {
                case psym_storage_class_specifier:
                        if (specs_info->storage_class != scs_none
                                && specs_info->storage_class != scs_typedef) {
                                SYMTABLE_ERROR(curr, MC_ERR, 
                                        "Multiple storage class specifiers");
                                status = MC_PARSE_ERROR;
                                break;
                        }
                        specs_info->storage_class
                                |= type_fetch_storage_class_specifier(curr);
                        break;
                case psym_type_specifier:
                        status = type_set_type_specifier(
                                t_info, curr, type_fetch_type_specifier(curr));
                        break;
                case psym_type_qualifier:
                        specs_info->type_qual 
                                |= type_fetch_type_qualifier(curr);
                        break;
                case psym_function_specifier:
                        specs_info->function_spec 
                                |= type_fetch_function_specifier(curr);
                        break;
                default:
                        SYMTABLE_ERROR(curr, MC_ERR, 
                                "Unexpected declaration specifier");
                        status = MC_PARSE_ERROR;
                        break;
                }
                if (!MC_SUCC(status))
                        return status;
                
        }
        if (t_info->base.type_spec == 0) {
                SYMTABLE_ERROR(decl_spec, MC_ERR, 
                                "No type specifier in declaration");
                status = MC_PARSE_ERROR;
        }

        return status;
}