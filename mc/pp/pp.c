#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <list.h>

#include <pp.h>


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


static inline _Bool pp_newline(char chr)
{
        return chr == '\n';
}

static inline _Bool pp_isspace(char chr)
{
        return isspace(chr) && !pp_newline(chr);
}

static void pp_skipspace(struct preproc *pp)
{
        const char *pos = pp->file_pos;
        while (*pp->file_pos != '\0') {
                if (*pos == '\\' && pp_newline(*(pos + 1))) {
                        pp->file_line += 1;
                        pos += 2;
                } else
                        break;

                if (pp_isspace(*pos)) {
                        pos += 1;
                        continue;
                }

                if (pp_newline(*pos)) {
                        pp->file_line += 1;
                        pp->line += 1;
                        pos += 1;
                        continue;
                }

                break;
        }
        pp->file_pos = pos;
}


_Bool pp_get_token(struct preproc *pp)
{
        _Bool status = false;
        const char *pos = pp->file_pos;

        pp_skipspace(pp);

        if (*pos == '<' || *pos == '"') {
                
                if (pp->last != NULL) {
                        struct pp_token *tok_inc = list_prev(pp->last);
                        if (tok_inc != NULL) {
                                struct pp_token *tok_hash = list_prev(tok_inc);
                                if (tok_hash != NULL && pp_token_valcmp(tok_inc, PP_VAL_INCLUDE) 
                                && pp_token_valcmp(tok_hash, PP_VAL_HASH) && tok_inc->line == pp->line
                                && tok_hash->line == pp->line) {
                                        const char *value = pos;
                                        size_t tf_line = pp->file_line; 

                                        char last_tok = '\0';
                                        if (*pos == '<')
                                                last_tok = '>';
                                        else
                                                last_tok = '"';
                                        // search for '>'
                                        while (*pos != last_tok || !(pp_newline(*pos)))
                                                pos += 1;
                                                // _next_position(pp)

                                        /* #include <hdr> matched */
                                        if (*pos == last_tok) {
                                                struct pp_token *token = malloc(sizeof(struct pp_token));
                                                token->type = pp_hdr_name;
                                                token->line = pp->line;
                                                token->file_line = tf_line;
                                                token->value = value;
                                                token->length = pos - token->value + 1;
                                                list_append(pp->last, token);
                                                pp->last = token;
                                                pp->file_pos = pos + 1;
                                                status = true;
                                        }
                                }
                        }
                }
        }

        return status;
}