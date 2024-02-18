#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <list.h>
#include <pp/lexer.h>
#include <pp/token.h>

/* Todo:
 * - replace pp->pos != '\0' with pp_eof inline */



static void pp_seterror(struct pp_lexer *pp)
{
        pp->state = false;
}


static void pp_advance(struct pp_lexer *pp, size_t steps)
{
        for ( ; steps > 0; pp->pos++, steps--) {
                assert(pp->pos[0] != '\0');
                if (pp->pos[0] == '\n') {
                        pp->file_line += 1;
                        if (!pp->_escape)
                                pp->line += 1;
                } else if (pp->pos[0] == '\\') {
                        pp->_escape = true;
                        continue;
                }
                pp->_escape = false;
        }
}


static inline _Bool pp_newline(const struct pp_lexer *pp)
{
        return pp->pos[0] == '\n';
}

static inline _Bool pp_isspace(const struct pp_lexer *pp)
{
        return isspace(pp->pos[0]) && !pp_newline(pp);
}


static inline _Bool pp_eof(const struct pp_lexer *pp)
{
        return pp->pos[0] == '\0';
}


static inline _Bool pp_isoctal(struct pp_lexer *pp)
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


static _Bool pp_isstr(struct pp_lexer *pp, const char *str)
{
        int slen = strlen(str);
        _Bool result = true;
        const char *pos = pp->pos;
        for (int i = 0; i < slen; i++)  {
                if (pos[i] == '\0') {
                        result = false;
                        break;
                }
                if (pos[i] != str[i]) {
                        result = false;
                        break;
                }
        }
        return result; 
}


static _Bool pp_is_one_of(char one, const char *of)
{
        int of_len = strlen(of);
        _Bool result = false;
        for (int i = 0; i < of_len; i++) {
                if (one == of[i]) {
                        result = true;
                        break;
                }
                        
        }
        return result;
}


static _Bool pp_skipspace(struct pp_lexer *pp)
{
        _Bool was_found = false;
        while (pp->pos[0] != '\0') {
                if (pp_isstr(pp, "\\\n")) {
                        pp_advance(pp, 2);
                        was_found = true;
                }
                if (pp_isspace(pp)) {
                        pp_advance(pp, 1);
                        was_found = true;
                        continue;
                }
                break;
        }
        return was_found;
}


static _Bool pp_skipcomment(struct pp_lexer *pp)
{
        _Bool was_found = false;
        if (pp_isstr(pp, "//")) {
                size_t curr_l = pp->line;
                while (curr_l == pp->line) {
                        assert(!pp_eof(pp)); /* anyway file ends in newline */
                        pp_advance(pp, 1);
                }
                was_found = true;
        } else if (pp_isstr(pp, "/*")) {
                pp_advance(pp, 2);
                while (!pp_isstr(pp, "*/") && !pp_eof(pp))
                        pp_advance(pp, 1);
                if (!pp_eof(pp)) {
                        pp_advance(pp, 2);
                        was_found = true;
                }
        }
        return was_found;
}


static void pp_header_name_unchecked(struct pp_lexer *pp)
{
        size_t start_l = pp->line;
        char last_tok = '\0';
        if (pp->pos[0] == '<')
                last_tok = '>';
        else
                last_tok = '"';

        // search for '>'
        char last = '\0';
        while ((last = pp->pos[0]) && last != last_tok)
                pp_advance(pp, 1);
        if (last == last_tok && pp->line == start_l)
                pp_advance(pp, 1);
        else
                pp_seterror(pp);
}


static void pp_fetch_header_name(struct pp_lexer *pp)
{
        if (*pp->pos == '<' || *pp->pos == '"') {
                if (pp->last != NULL) {
                        struct pp_token *tok_inc = pp->last;
                        if (tok_inc != NULL) {
                                struct pp_token *tok_hash = list_prev(tok_inc);

                                if (tok_hash != NULL 
                                && pp_token_valcmp(tok_inc, PP_VAL_INCLUDE) == 0
                                && pp_token_valcmp(tok_hash, PP_VAL_HASH) == 0 
                                && tok_inc->line == pp->line
                                && tok_hash->line == pp->line)
                                        pp_header_name_unchecked(pp);
                        }
                }       
        } else
                pp_seterror(pp);
}


static void pp_fetch_univ_char_name(struct pp_lexer *pp)
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


static void pp_fetch_id_ndigit(struct pp_lexer *pp)
{

        if (pp_nondigit(*pp->pos)) {
                pp_advance(pp, 1);
                return;
        }

        /* universal char name */
        pp_fetch_univ_char_name(pp);
        if (pp_lexer_noerror(pp))
                return;
                
        pp_seterror(pp);
}


static void pp_fetch_number(struct pp_lexer *pp)
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

                        struct pp_lexer pp_save = *pp;
                        pp_fetch_id_ndigit(pp);
                        if (pp_lexer_noerror(pp))
                                continue;
                        else
                                *pp = pp_save;

                        break;
                }
        } else
                pp_seterror(pp);
}


static void pp_fetch_identifier(struct pp_lexer *pp)
{
        pp_fetch_id_ndigit(pp);
        if (pp_lexer_noerror(pp))
                while (*pp->pos != '\0') {

                        struct pp_lexer pp_save = *pp;
                        pp_fetch_id_ndigit(pp);
                        if (pp_lexer_noerror(pp))
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


static void pp_fetch_escape_seq(struct pp_lexer *pp)
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
        if (pp_lexer_noerror(pp))
                return;

        pp_seterror(pp);
}


static void pp_fetch_quotation(struct pp_lexer *pp, char qchar)
{
        if (*pp->pos == qchar || (*pp->pos == 'L' && *(pp->pos + 1) == qchar)) {
                pp_advance(pp, (*pp->pos == 'L') ? 2 : 1);

                while (pp->pos[0] != qchar) {

                        if (pp->pos[0] == '\\' && pp->pos[1] == '\n')
                                pp_advance(pp, 2);

                        struct pp_lexer pp_save = *pp;
                        pp_fetch_escape_seq(pp);
                        if (pp_lexer_noerror(pp))
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


static inline void pp_fetch_strlit(struct pp_lexer *pp)
{
        pp_fetch_quotation(pp, '"');
}


static inline void pp_fetch_chrconst(struct pp_lexer *pp)
{
        pp_fetch_quotation(pp, '\'');
}


static inline void pp_fetch_punctuator(struct pp_lexer *pp)
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

/* Take caution, we may leak ptr here */
static void pp_lexer_reset(struct pp_lexer *pp)
{
        pp->first = NULL;
        pp->last = NULL;
        pp->line = 0;
        pp->file_line = 0;
        pp->state = true;
        pp->_escape = false;
}


_Bool pp_lexer_init(struct pp_lexer *pp, struct fs_file *file)
{
        if (file->content == NULL)
                pp->pos = source_read(file);
        if (pp->pos == NULL)
                return false;
        pp->src_file = file;
        pp_lexer_reset(pp);

        return true;
}

struct pp_token *pp_lexer_result(struct pp_lexer *pp)
{
        struct pp_token *result = pp->first;
        pp_lexer_reset(pp);
        return result;
}


void pp_lexer_add_token(struct pp_lexer *pp, struct pp_token *token)
{
        assert(token != NULL);
        if (pp->last != NULL) {
                list_append(pp->last, token);
                pp->last = token;
        } else {
                pp->first = token;
                pp->last = token;
        }
}


_Bool pp_lexer_noerror(const struct pp_lexer *pp)
{
        return pp->state == true;
}



void pp_lexer_free(struct pp_lexer *pp)
{
        if (pp->first != NULL)
                pp_token_list_destroy(pp->first);
}


struct pp_token *pp_lexer_get_token(struct pp_lexer *pp)
{
        struct pp_token *token = NULL;

        while (pp_skipspace(pp) && pp_skipcomment(pp));
        struct pp_lexer pp_save = *pp;

        /* header name */
        pp_fetch_header_name(pp);
        token = pp_token_create(&pp_save, pp, pp_hdr_name);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;
        
        pp_fetch_chrconst(pp);
        token = pp_token_create(&pp_save, pp, pp_chr_const);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        pp_fetch_strlit(pp);
        token = pp_token_create(&pp_save, pp, pp_str_lit);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        // identifier
        pp_fetch_identifier(pp);
        token = pp_token_create(&pp_save, pp, pp_id);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        /* pp-number */
        pp_fetch_number(pp);
        token = pp_token_create(&pp_save, pp, pp_number);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        pp_fetch_punctuator(pp);
        token = pp_token_create(&pp_save, pp, pp_punct);
        if (token == NULL)
                *pp = pp_save;
        else
                return token;

        if (pp->pos[0] != '\0') {
                pp_advance(pp, 1);
                return pp_token_create(&pp_save, pp, pp_other);
        }
        
        return NULL;
}
