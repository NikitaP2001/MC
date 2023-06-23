#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <list.h>

#include <pp.h>

/* Todo:
 *      - replace *pp->pos... usage to pp-pos[x] 
 *      - change pp_is* functions to use pp instead of raw * */


static struct pp_token *pp_try_form_token(const struct preproc *before, 
const struct preproc *after, enum pp_type type)
{
        struct pp_token *token = NULL;
        if (before->line == after->line && before->pos != after->pos 
        && pp_noerror(after)) {
                token = malloc(sizeof(struct pp_token));
                token->file_line = after->file_line;
                token->line = after->line;
                token->length = after->pos - before->pos + 1;
                token->value = before->pos;
                token->type = type;
        }
        return token;
}


int pp_token_valcmp(struct pp_token *left, const char *value)
{
        size_t r_len = strlen(value);
        if (r_len > left->length)
                return -1;
        else if (r_len < left->length)
                return 1;
        else
                return strncmp(left->value, value, left->length);
}


static void pp_seterror(struct preproc *pp)
{
        pp->state = false;
}


static void pp_advance(struct preproc *pp, size_t steps)
{
        pp->pos += steps;
}


static inline _Bool pp_newline(char chr)
{
        return chr == '\n';
}

static inline _Bool pp_isspace(char chr)
{
        return isspace(chr) && !pp_newline(chr);
}


static inline _Bool pp_isoctal(struct preproc *pp)
{
        return pp->pos[0] >= '0' && pp->pos[0] <= '7';
}


static inline _Bool pp_ishex(char chr)
{
        return isdigit(chr) || (chr >= 'A' && chr <= 'F')
        || (chr >= 'a' && chr <= 'f');
}

static inline _Bool pp_nondigit(char chr)
{
        return isalpha(chr) || chr == '_';
}


static _Bool pp_isstr(struct preproc *pp, const char *str)
{
        int slen = strlen(str);
        const char *pos = pp->pos;
        for (int i = 0; i < slen && pos[i] != '\0'; i++)  {
                if (pos[i] != str[i])
                        return false;
        }
        return true; 
}


static _Bool pp_is_one_of(char one, const char *of)
{
        int of_len = strlen(of);
        for (int i = 0; i < of_len; i++) {
                if (one != of[i])
                        return false;   
        }
        return true; 
}


static void pp_skipspace(struct preproc *pp)
{
        while (*pp->pos != '\0') {
                if (*pp->pos == '\\' && pp_newline(*(pp->pos + 1))) {
                        pp_advance(pp, 2);
                } else
                        break;

                if (pp_isspace(*pp->pos)) {
                        pp_advance(pp, 1);
                        continue;
                }

                break;
        }
}



static void pp_fetch_header_name(struct preproc *pp)
{
        if (*pp->pos == '<' || *pp->pos == '"') {
                if (pp->last != NULL) {
                        struct pp_token *tok_inc = list_prev(pp->last);
                        if (tok_inc != NULL) {
                                struct pp_token *tok_hash = list_prev(tok_inc);
                                if (tok_hash != NULL && pp_token_valcmp(tok_inc, PP_VAL_INCLUDE) 
                                && pp_token_valcmp(tok_hash, PP_VAL_HASH) && tok_inc->line == pp->line
                                && tok_hash->line == pp->line) {

                                        char last_tok = '\0';
                                        if (pp->pos[0] == '<')
                                                last_tok = '>';
                                        else
                                                last_tok = '"';

                                        // search for '>'
                                        char last = '\0';
                                        while ((last = *pp->pos) && last != last_tok)
                                                pp_advance(pp, 1);
                                        if (last != last_tok)
                                                pp_seterror(pp);
                                }
                        }
                }       
        } else
                pp_seterror(pp);
}


static void pp_fetch_univ_char_name(struct preproc *pp)
{
        if (pp->pos[0] == '\\' && toupper(pp->pos[1]) == 'U') {
                int ndigits = (pp->pos[1] == 'U') ? 8 : 4;
                pp_advance(pp, 2);
                while (ndigits > 0) {
                        if (!pp_ishex(*pp->pos)) {
                                pp_seterror(pp);
                                break;
                        }
                        pp_advance(pp, 1);
                        ndigits -= 1;
                }
                if (ndigits == 0)
                        return;
        }

        pp_seterror(pp);
}


static void pp_fetch_id_ndigit(struct preproc *pp)
{

        if (pp_nondigit(*pp->pos)) {
                pp_advance(pp, 1);
                return;
        }

        /* universal char name */
        pp_fetch_univ_char_name(pp);
        if (pp_noerror(pp))
                return;
                
        pp_seterror(pp);
}


static void pp_fetch_number(struct preproc *pp)
{
        if (isdigit(*pp->pos) || (*pp->pos == '.' && isdigit(*(pp->pos + 1)))) {
                pp_advance(pp, (*pp->pos == '.') ? 2 : 1);
                while (*pp->pos != '\0') {

                        // digit
                        if (isdigit(*pp->pos) || *pp->pos == '.') {
                                pp_advance(pp, 1);
                                continue;
                        }

                        if ((toupper(*pp->pos) == 'P' || toupper(*pp->pos) == 'E')
                        && (*(pp->pos + 1) == '-' || *(pp->pos + 1) == '+')) {
                                pp_advance(pp, 1);
                                continue;
                        }

                        pp_fetch_id_ndigit(pp);
                        if (pp_noerror(pp))
                                continue;

                        break;
                }
        } else
                pp_seterror(pp);
}


static void pp_fetch_identifier(struct preproc *pp)
{
        pp_fetch_id_ndigit(pp);
        if (pp_noerror(pp))
                while (*pp->pos != '\0') {

                        struct preproc pp_save = *pp;
                        pp_fetch_id_ndigit(pp);
                        if (pp_noerror(pp))
                                continue;
                        else
                                *pp = pp_save;
                        
                        if (isdigit(*pp->pos)) {
                                pp_advance(pp, 1);
                                continue;
                        }

                        break;
                }
        else
                pp_seterror(pp);
}


static void pp_fetch_escape_seq(struct preproc *pp)
{
        if (pp->pos[0] == '\\') {
                pp_advance(pp, 1);
                if (pp_is_one_of(*pp->pos, "'\"?\\abfnrtv")) {
                        pp_advance(pp, 1);
                        return;
                }

                if (pp_isoctal(pp)) {
                        pp_advance(pp, 1);
                        if (pp_isoctal(pp))
                                pp_advance(pp, 1);
                        if (pp_isoctal(pp))
                                pp_advance(pp, 1);
                        return;
                }

                if (pp->pos[0] == 'x') {
                        if (pp_ishex(*pp->pos)) {
                                pp_advance(pp, 1);
                                while (pp_ishex(*pp->pos))
                                        pp_advance(pp, 1);
                                return;
                        }
                }
        }

        pp_fetch_univ_char_name(pp);
        if (pp_noerror(pp))
                return;

        pp_seterror(pp);
}


static void pp_fetch_quotation(struct preproc *pp, char qchar)
{
        if (*pp->pos == qchar || (*pp->pos == 'L' && *(pp->pos + 1) == qchar)) {
                pp_advance(pp, (*pp->pos == 'L') ? 2 : 1);

                while (pp->pos[0] != qchar) {

                        struct preproc pp_save = *pp;
                        pp_fetch_escape_seq(pp);
                        if (pp_noerror(pp))
                                continue;
                        else
                                *pp = pp_save;

                        if (pp->pos[0] == '\\' || pp->pos[0] == '\0') {
                                pp_seterror(pp);
                                return;
                        }

                        pp_advance(pp, 1);
                } 

                pp_advance(pp, 1);

        } else
                pp_seterror(pp);
}


static inline void pp_fetch_strlit(struct preproc *pp)
{
        pp_fetch_quotation(pp, '\'');
}


static inline void pp_fetch_chrconst(struct preproc *pp)
{
        pp_fetch_quotation(pp, '"');
}


static inline void pp_fetch_punctuator(struct preproc *pp)
{
        if (pp_is_one_of(pp->pos[0], "[](){}~,;?")) {
                pp_advance(pp, 1);
                return;
        }

        static const char *str_punct[] = {
                "%:%:", "...", "<<=", ">>=",

                "->", "++", "--", "<<", ">>",
                "<=", ">=", "==", "!=", "&&",
                "||", "*=", "/=", "%=", "+=",
                "-=", "&=", "^=", "|=", "##",
                "<:", ">:", "<%", "%>", "%:",
                
                ".", "!", "-", "+", "*", "&",
                "<", ">", "^", "|", ":", "=",
                "#", ","
        };

        for (size_t pi = 0; pi < sizeof(str_punct) / sizeof(char *); pi++) {
                if (pp_isstr(pp, str_punct[pi])) {
                        pp_advance(pp, strlen(str_punct[pi]));
                        return;
                }
        }

        pp_seterror(pp);
}


_Bool pp_init(struct preproc *pp, struct fs_file *file)
{
        if (file->content == NULL)
                pp->pos = source_read(file);
        if (pp->pos == NULL)
                return false;

        pp->first = NULL;
        pp->last = NULL;
        pp->line = 0;
        pp->file_line = 0;
        pp->state = true;

        return true;
}


_Bool pp_noerror(const struct preproc *pp)
{
        return pp->state == true;
}



void pp_free(struct preproc *pp)
{
        if (pp->first != NULL)
                list_destroy(pp->first, free);
}


struct pp_token *pp_get_token(struct preproc *pp)
{
        struct pp_token *token = NULL;

        pp_skipspace(pp);

        struct preproc pp_save = *pp;

        /* header name */
        pp_fetch_header_name(pp);
        token = pp_try_form_token(&pp_save, pp, pp_hdr_name);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;
        
        pp_fetch_chrconst(pp);
        token = pp_try_form_token(&pp_save, pp, pp_chr_const);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        pp_fetch_strlit(pp);
        token = pp_try_form_token(&pp_save, pp, pp_str_lit);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        // identifier
        pp_fetch_identifier(pp);
        token = pp_try_form_token(&pp_save, pp, pp_id);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        /* pp-number */
        pp_fetch_number(pp);
        token = pp_try_form_token(&pp_save, pp, pp_number);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        pp_fetch_number(pp);
        token = pp_try_form_token(&pp_save, pp, pp_punct);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        if (pp->pos[0] != '\0') {
                pp_advance(pp, 1);
                return pp_try_form_token(&pp_save, pp, pp_other);
        }
        
        return NULL;
}
