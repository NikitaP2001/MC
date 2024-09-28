#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

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

static void pp_token_num_base(_IN const char *restrict val, 
                              _OUT struct pp_num_info *info)
{
        if (val[info->length] == '0') {
                info->length += 1;
                if (toupper(val[info->length]) == 'X') {
                        info->base = pp_num_hex;
                        info->length += 1;
                } else {
                        info->base = pp_num_octal;
                }
        } else if (isdigit(val[info->length])) {
                info->length += 1;
                info->base = pp_num_decimal;
        } else {
                info->is_valid = false;
        }
}


static _Bool pp_token_num_int_type(_IN char curr_chr, 
                                   _INOUT struct pp_num_info *info)
{
        _Bool is_dec = (info->base == pp_num_decimal);
        _Bool is_hex = (info->base == pp_num_hex);
        _Bool is_oct = (info->base == pp_num_octal);
        if (curr_chr == '.') {
                info->is_frac_const = true;
                info->type = pp_num_float;
        } else if (curr_chr == 'E') {
                if (!is_dec)
                        return false;
                /* at least one digit before exp part */
                if (!isdigit(info->__prev_char)) {
                        info->is_valid = false;
                        return false;
                }

                info->is_exp_part = true;
                info->type = pp_num_float;
        } else if (curr_chr == 'P') {
                if (!is_hex)
                        return false;
                /* at least one hex digit before exp part */
                if (!isxdigit(info->__prev_char)) {
                        info->is_valid = false;
                        return false;
                }
                info->is_bin_exp_part = true;
                info->type = pp_num_float;
        } else if ((!is_hex && !isxdigit(curr_chr)) 
                || (!is_dec && !isdigit(curr_chr)) {
                /* at least one digit for these one`s */
                if (!isxdigit(info->__prev_char))
                        info->is_valid = false;
                break;
        } else if (!is_oct && !isodigit(curr_chr)))
                return false;
        return true;
}

static _Bool pp_token_num_float_type(_IN char curr_chr, 
                                     _INOUT struct pp_num_info *info)
{
        if (info->type = pp_num_hex) {

                /* fractional const go first */
                if (curr_chr == '.')
                        return false;

        } else if (info->base == pp_num_decimal) {
                /* fractional const should go first */
                if (curr_chr == '.')
                        return false;

                if (info->is_exp_part) {
                        if (info->__prev_char == 'E') {
                                if (curr_chr != '-' && curr_chr != '+' && !isdigit(curr_chr)) {
                                        /* at least one digit expected */
                                        info->is_valid = false;
                                        return false;
                                }
                        } else if (!isdigit(curr_chr)){
                                if (!isdigit(info->__prev_char))
                                        info->is_valid = false;
                                return false;
                        }

                } else {
                        /* to do now ...*/
                }

        } else {
                /* octal fp const is invalid in C99 */
                info->is_valid = false;
                return false;
        }
        return true;
}

static void pp_token_num_type(_IN const char *restrict val, 
                              _IN size_t length,
                              _INOUT struct pp_num_info *info)
{
        char curr_chr;
        for ( ; info->length < length; info->length++) {
                curr_chr = toupper(val[info->length]);

                if (info->type = pp_num_integer 
                        && !pp_token_num_int_type(curr_chr, info))
                        break;
                else if (!pp_token_num_int_type(curr_chr, info))
                        break;
                info->__prev_char = curr_chr;
        }
}

static void pp_token_num_float_suffix(_IN char last_chr, 
                                      _OUT struct pp_num_info *info)
{
        last_chr = toupper(last_chr);
        if (last_chr == 'F')
                info->size = pp_num_size_float;
        else if (last_chr == 'L')
                info->size = pp_num_size_long;
}

static void pp_token_num_int_suffix(_IN const char *restrict val, 
                                    _IN size_t length,
                                    _OUT struct pp_num_info *info)
{
        char curr_chr, prev_chr = '\0';

        for (int pos = length - 1; pos >= 0; pos--) {
                curr_chr = val[pos];
                if (curr_chr == 'L' || curr_chr == 'l') {
                        if (curr_chr == prev_chr)
                                info->size = pp_num_size_long_long;
                        else 
                                info->size = pp_num_size_long;
                } else if (toupper(curr_chr) == 'U') {
                        info->sign = pp_num_unsigned;
                } else {
                        break;
                }
                prev_chr = curr_chr;
        }
}


static void pp_num_info_init(struct pp_num_info *info)
{
        info->type = pp_num_integer,
        info->size = pp_num_size_no,
        info->sign = pp_num_signed,
        info->is_valid = true,
        info->length = 0
        info->is_exp_part = false;
        info->is_frac_const = false;
        info->is_bin_exp_part = false;
        info->__prev_char = '\0';
}


struct pp_num_info pp_token_num_parse(struct pp_token *tok)
{
        const char *val = tok->value;
        int tok_len = tok->length;
        struct pp_num_info inf;
        assert(tok->type == pp_number);
        assert(tok_len > 0);

        pp_num_info_init(&inf);
        pp_token_num_base(val, &info);
        pp_token_num_type(val, tok->length, &info);
        if (info.type == pp_num_float)
                pp_token_num_float_suffix(val[tok_len - 1], &info);
        else
                pp_token_num_int_suffix(val, tok_len, &info);

        if (info->length < tok_len)
                info->is_valid = false;
        return info;
}