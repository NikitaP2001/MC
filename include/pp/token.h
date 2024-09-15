#ifndef _PP_TOKEN_H_
#define _PP_TOKEN_H_
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <list.h>
#include <mc.h>

enum pp_type {
        pp_hdr_name,
        pp_id,
        pp_number,
        pp_chr_const,
        pp_str_lit,
        pp_punct,
        pp_other
};

#define PP_VAL_INCLUDE "include"
#define PP_VAL_HASH "#"

typedef file_size_t line_num_t;

struct pp_token {
        struct list_head link;

        /* value is allocated while reading a source file
         * using the fs module, and residue in mem, until we 
         * free the fs module 
         * TODO: store file pos instead of pointer */
        char *value;
        file_size_t length;

        /* counts backslashes also, for internal use */
        line_num_t line;

        /* only counts new line chars, a real line */
        size_t file_line;

        struct fs_file *src_file;

        enum pp_type type;
};

struct pp_lexer;

struct pp_token *pp_token_create(const struct pp_lexer *before, 
const struct pp_lexer *after, enum pp_type type);

int pp_token_valcmp(struct pp_token *left, const char *value);

void pp_token_getval(struct pp_token *token, char *buffer);

static inline 
line_num_t
pp_token_line(struct pp_token *token)
{
        return token->line;
}

void pp_token_destroy(struct pp_token *token);

static inline void
pp_token_list_destroy(struct pp_token *token)
{
        list_destroy(token, (list_free)pp_token_destroy);
}

enum pp_num_size {
        pp_num_size_no,
        pp_num_size_float,
        pp_num_size_long,
        pp_num_size_long_long
};

enum pp_num_sign {
        pp_num_signed,
        pp_num_unsigned
};

enum pp_num_base {
        pp_num_octal = 8,
        pp_num_decimal = 10,
        pp_num_hex = 16
};

enum pp_num_type {
        pp_num_float,
        pp_num_integer
};

struct pp_num_info {
        enum pp_num_type type;
        enum pp_num_base base;
        enum pp_num_size size;
        enum pp_num_sign sign;
};

/* in one quick pass tries to induce type of num constatnt 
 * note: it does not validate const format */
struct pp_num_info pp_token_num_parse(struct pp_token *tok);

static inline
_Bool
pp_token_is_newline(struct pp_token *token)
{
        return token->type == pp_other && (pp_token_valcmp(token, "\n") == 0);
}

#endif /* _PP_TOKEN_H_ */