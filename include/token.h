#ifndef _TOKEN_H_
#define _TOKEN_H_
#include <mc.h>
#include <pp.h>
#include <tools.h>
#include <token/convert.h>

/* escape type do not match expected, but, in general,
 * is valid. */
#define TOKEN_ESCAPE_UNMATCH -2
/* general parse error, we will abort translation*/
#define TOKEN_ERROR -1

#define TOKEN_SUCC(status) (status >= 0)

#define TOKEN_STRL_MIN_LEN 2
#define TOKEN_WSTRL_MIN_LEN 3

#define TOKEN_OCTAL_BASE 8
#define TOKEN_OCTAL_ESC_LEN 3
#define TOKEN_HEX_BASE 0x10
#define TOKEN_HEX_ESC_LEN 8
/* maximum characters for single escape chars */
#define TOKEN_ESC_VAL_MAX_LEN 4

enum constant_type {

        /* integer-constant */
        const_invalid = 0,

        const_int = 1,
        const_long_int = (1 << 3),
        const_long_long_int = (1 << 5),

        const_uint = (1 << 2),
        const_ulong_int = (1 << 4),
        const_ulong_long_int = (1 << 6),

        /* floating-constant */
        const_float,
        const_double,
        const_long_double,

        /* int type */
        const_char,
        /* could be represented by int */
        const_wchar_t

};

static inline _Bool token_const_is_float(enum constant_type type)
{
        return type >= const_float && type <= const_long_double;
}

static inline _Bool token_const_is_integer(enum constant_type type)
{
        return type >= const_int && type <= const_ulong_long_int;
}

static inline _Bool token_const_is_char(enum constant_type type)
{
        return type >= const_char && type <= const_wchar_t;
}

extern const char token_simple_esc_chars[];

struct constant_value {
        enum constant_type type;
        union {
                int64_t var_int;
                uint64_t var_uint;
                long double var_long_double;
        } data;
};

/* note: keep token_type_strs consistent */
enum token_type {
        tok_keyword,
        tok_identifier,
        tok_constant,
        tok_strlit,
        tok_punctuator
};

extern const char* token_keywords[];

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
        keyw_union,
        keyw_last = keyw_union,

        keyw_invalid
};

static inline const char *token_keyw_to_str(enum keyword_type type)
{
        return token_keywords[type];
}

#define TOKEN_MAX_PUNC_LEN 4

extern struct trie_node token_punc_trie;
extern const char *token_punctuators[];

enum punc_type {
        punc_digraph_double_hash,
        punc_dots,
        punc_shl_assign,
        punc_shr_assigh,
        
        punc_right_arrow,
        punc_increment,
        punc_decrement,
        punc_shl,
        punc_shr,
        punc_less_eq,
        punc_greater_eq,
        punc_equal,
        punc_not_equal,
        punc_logical_and,
        punc_logical_or,
        punc_mul_assign,
        punc_div_assign,
        punc_mod_assign,
        punc_add_assign,
        punc_sub_assign,
        punc_and_assign,
        punc_xor_assign,
        punc_or_assign,
        punc_double_hash,
        /* TODO: implement digraph support */
        punc_digraph_left_sq_br,
        punc_digraph_right_sq_br,
        punc_digraph_left_ql_br,
        punc_digraph_right_ql_br,
        punc_digraph_hash,

        punc_dot,
        punc_exclamation,
        punc_sub,
        punc_add,
        punc_mul,
        punc_bit_and,
        punc_less,
        punc_greater,
        punc_xor,
        punc_bar,
        punc_colon,
        punc_assign,
        punc_hash,
        punc_comma,
        punc_semicolon,
        punc_question,
        punc_left_sq_br,
        punc_right_sq_br,
        punc_left_rnd_br,
        punc_right_rnd_br,
        punc_left_ql_br,
        punc_right_ql_br,
        punc_tilde,
        punc_forward_slash,
        punc_percent,
        punc_last = punc_percent,

        punc_invalid,
};

static inline const char *token_punc_to_str(enum punc_type type)
{
        return token_punctuators[type];
}

union token_value {

        enum keyword_type var_keyw;
        enum punc_type var_punc;

        struct {
                char *value;
                file_size_t length;
                _Bool is_alloc;
        } var_raw;

        struct constant_value var_const;
        
};

/* TODO: replace linked list with just array, 
 * then also store pp tok ptr in seperate place */
struct token {
        struct list_head link;
        enum token_type type;

        union token_value value; 

        /* first preprocessing token, which
         * was met, while converting this token */
        struct pp_token *first;
};

void token_destroy(struct token *tok);

struct token* token_convert_next(struct convert_context *ctx);

/* one time initialization of the token module */
void token_global_init();

void token_global_free();

void token_print(struct token *tok);

#ifdef DEBUG
void token_print_content(struct token *tok);

#define TOKEN_PRINT_CONTENT(tok) token_print_content(tok)

#else /* DEBUG */

#define TOKEN_PRINT_CONTENT(tok) do {} while (false)

#endif /* DEBUG */

#endif /* _TOKEN_H_ */