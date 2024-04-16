#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <pp/token.h>
#include <pp/lexer.h>

static void 
pp_token_set_value(struct pp_token *token, const struct pp_lexer *before)
{
        _Bool no_skip_chr = true;
        for (size_t i = 0; i < token->length; i++) {
                if (before->pos[i] == '\\' && before->pos[i + 1] == '\n') {
                        no_skip_chr = false;
                        break;
                }
        }
        if (!no_skip_chr) {
                token->value = calloc(token->length, sizeof(char));
                size_t ival = 0;
                size_t iorig = 0;
                while (iorig < token->length) {
                        if (before->pos[iorig] == '\\' 
                        && before->pos[iorig + 1] == '\n') {
                                iorig += 2;
                                continue;
                        }
                        token->value[ival++] = before->pos[iorig++];
                }
                token->length = ival;
        }
}


struct pp_token *pp_token_create(const struct pp_lexer *before, 
const struct pp_lexer *after, enum pp_type type)
{
        struct pp_token *token = NULL;

        if (before->pos != after->pos && pp_lexer_noerror(after)) {

                token = calloc(1, sizeof(struct pp_token));
                token->file_line = after->file_line;
                token->line = after->line;
                token->length = after->pos - before->pos;
                token->value = (char *)before->pos;
                token->type = type;
                token->src_file = fs_share_file(before->src_file);
                assert(before->src_file == after->src_file);

                pp_token_set_value(token, before);
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


void pp_token_getval(struct pp_token *token, char *buffer)
{
        strncpy(buffer, token->value, token->length);
        buffer[token->length] = '\0';
}


void pp_token_destroy(struct pp_token *token)
{
        fs_release_file(token->src_file);
        free(token);
}