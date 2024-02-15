#ifndef _PP_TOKEN_H_
#define _PP_TOKEN_H_
#include <stddef.h>
#include <stdbool.h>

#include <list.h>

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

struct pp_token {
        struct list_head link;

        char *value;
        size_t length;

        enum pp_type type;

        /* counts backslashes also */
        size_t line;

        /* only counts new line chars */
        size_t file_line;
};

struct pp_lexer;

struct pp_token *pp_token_create(const struct pp_lexer *before, 
const struct pp_lexer *after, enum pp_type type);

int pp_token_valcmp(struct pp_token *left, const char *value);

void pp_token_getval(struct pp_token *token, char *buffer);

void pp_token_destroy(struct pp_token *token);

/* reading next characted from token list */
char pp_token_read_next(struct pp_token **token, uint32_t *pos);

#endif /* _PP_TOKEN_H_ */