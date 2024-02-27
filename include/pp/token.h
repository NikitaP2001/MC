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

/* reading next characted from token list */
char pp_token_read_next(struct pp_token **token, uint32_t *pos);

#endif /* _PP_TOKEN_H_ */