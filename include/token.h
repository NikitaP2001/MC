#ifndef _TOKEN_H_
#define _TOKEN_H_
#include <mc.h>
#include <pp.h>

enum constant_type {

        /* integer-constant */
        const_short_int,
        const_ushort_int,

        const_int,
        const_long_int,
        const_uint,
        const_ulong_int,

        /* floating-constant */
        const_float,
        const_double,
        const_long_double,

        const_enum,

        /* int type */
        const_char,
        /* could be represented by int */
        const_wchar_t

};

struct constant_value {
        enum constant_type type;
        union {
                uint64_t var_uint;
                int64_t var_int;
                float var_float;
                double var_double;
                long double var_long_double;
        } data;
};

enum token_type {
        tok_keyword,
        tok_identifier,
        tok_constant,
        tok_strlit,
        tok_punctuator
};

enum keyword_type {
        keyw_auto,
        keyw_enum,
        keyw_restrict,
        keyw_break,
        keyw_extern,
        keyw_return,
        keyw_unsigned,
        keyw_void,
        keyw_case,
        keyw_float,
        keyw_short,
        keyw_char,
        keyw_for,
        keyw_signed,
        keyw_volatile,
        keyw_while,
        keyw_const,
        keyw_goto,
        keyw_sizeof,
        keyw_Bool,
        keyw_continue,
        keyw_if,
        keyw_static,
        keyw_Complex,
        keyw_default,
        keyw_inline,
        keyw_struct,
        keyw_Imaginary,
        keyw_do,
        keyw_int,
        keyw_switch,
        keyw_double,
        keyw_long,
        keyw_typedef,
        keyw_else,
        keyw_register,
        keyw_union
};

union token_value {

        enum keyword_type var_keyw;

        struct {
                _Bool alloc_value;
                char *value;
                file_size_t length;
        } var_raw;

        struct constant_value var_const;
        
};



struct token {
        struct list_head link;
        enum token_type type;

        union token_value value; 

        /* first preprocessing token, which
         * was met, while converting this token */
        struct pp_token *first;

};

/* creates token, which value is based on preprocessing token range 
 * Also, convertation to execution character is performed */
struct token *token_create(struct pp_token *first, struct pp_token *last);

void token_destroy(struct token *tok);

/* this implements convertation described in C99 standart 
 * with the following steps:
 *
 * Each source character set member and escape sequence 
 * in character constants and string literals is converted 
 * to the corresponding member of the execution character 
 * set; if there is no corresponding member, it is converted
 * to an implementation-defined member other than the null 
 * (wide) character.
 *
 * Adjacent string literal tokens are concatenated. 
 * 
 * White-space characters separating tokens are no longer significant. Each 
 * preprocessing token is converted into a token. */
struct token *token_convert(struct pp_context *pp);

#endif /* _TOKEN_H_ */