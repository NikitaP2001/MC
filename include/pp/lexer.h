#ifndef _PP_LEXER_
#define _PP_LEXER_
#include <fs.h>

struct pp_lexer {
        const char *pos;
        size_t line;
        size_t file_line;

        _Bool state;

        struct pp_token *first;
        struct pp_token *last;

        _Bool _escape;

        struct fs_file *src_file;
};

struct pp_token;

_Bool pp_lexer_init(struct pp_lexer *pp, struct fs_file *file);

/* Returns an ownership for the result */
struct pp_token *pp_lexer_move(struct pp_lexer *pp);

/* returns the result, with no ownership */
const struct pp_token *pp_lexer_get(struct pp_lexer *pp);

_Bool pp_lexer_noerror(const struct pp_lexer *pp);

void pp_lexer_run(struct pp_lexer *pp);

void pp_lexer_free(struct pp_lexer *pp);

/* @return number of char matched, starting from 1 */
int pp_is_one_of(char one, const char *of);

#endif /* _PP_LEXER_ */