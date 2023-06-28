#include <list.h>

#include <fs.h>

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

        _Bool _is_val_orig;
        char *value;
        size_t length;

        enum pp_type type;

        /* counts backslashes also */
        size_t line;

        /* only counts new line chars */
        size_t file_line;
};

int pp_token_valcmp(struct pp_token *left, const char *value);


void pp_token_getval(struct pp_token *token, char *buffer);


struct preproc {
        const char *pos;
        size_t line;
        size_t file_line;

        _Bool state;

        struct pp_token *first;
        struct pp_token *last;

        _Bool _escape;
};

_Bool pp_init(struct preproc *pp, struct fs_file *file);

void pp_add_token(struct preproc *pp, struct pp_token *token);

_Bool pp_noerror(const struct preproc *pp);

struct pp_token *pp_get_token(struct preproc *pp);

void pp_free(struct preproc *pp);