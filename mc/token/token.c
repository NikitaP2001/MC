#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <math.h>

#include <tools.h>
#include <token.h>
#include <pp/node.h>
#include <pp/token.h>

#define STRLIT_START_OFFSET 1
#define WSTRLIT_START_OFFSET 2

static struct token *token_create(struct pp_token *first)
{
        struct token *tok = malloc(sizeof(struct token));
        memset(tok, 0, sizeof(struct token));
        tok->first = first;
        return tok;
}

void token_destroy(struct token *tok)
{
        if (tok->value.var_raw.is_alloc)
                free(tok->value.var_raw.value);
        free(tok);
}

/* order should match enum token_type */
const char* token_keywords[] = {
        "auto",
        "enum",
        "restrict",
        "break",
        "extern",
        "return",
        "unsigned",
        "void",
        "case",
        "float",
        "short",
        "char",
        "for",
        "signed",
        "volatile",
        "while",
        "const",
        "goto",
        "sizeof",
        "_Bool",
        "continue",
        "if",
        "static",
        "_Complex",
        "default",
        "inline",
        "struct",
        "_Imaginary",
        "do",
        "int",
        "switch",
        "double",
        "long",
        "typedef",
        "else",
        "register",
        "union"
};

static struct trie_node token_keyw_trie;

static inline _Bool token_is_wstr(struct pp_token *strlit)
{
        return strlit->value[0] == 'L';
}

const char token_simple_esc_chars[] = "'\"?\\abfnrtv";
const char token_simple_esc_vals[] = "\'\"\?\\\a\b\f\n\r\t\v";

static int64_t token_simple_esc_value(const char *value, size_t *pos)
{
        int64_t result = TOKEN_ESCAPE_UNMATCH;
        size_t n_pos = *pos, n_val;
        if (value[n_pos] != '\\')
                return result;
        n_pos += 1;
        if ((n_val = pp_is_one_of(value[n_pos], token_simple_esc_chars))) {
                assert(n_val < sizeof(token_simple_esc_vals));
                result = token_simple_esc_vals[n_val - 1];
        }
        if (result != TOKEN_ESCAPE_UNMATCH)
                *pos = n_pos + 1;
        return result;
}

static _Bool token_is_octal(char c)
{
        return c >= '0' && c < '8';
}

static _Bool token_is_hex(char c)
{
        return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static inline int token_oct_to_int(char chr)
{
        return chr - '0';
}

static inline int token_hex_to_int(char chr)
{
        if (chr > '9')  {
                if (chr >= 'a')
                        return 0xA + chr - 'a';
                else
                        return 0xA + chr - 'A';
        } else
                return chr - '0';
}

static int64_t token_octal_esc_value(const char *value, size_t *pos)
{
        int64_t result = TOKEN_ESCAPE_UNMATCH;
        size_t n_chr = 0;
        char c;

        for ( ; token_is_octal(c = value[*pos + n_chr + 1]); n_chr++) {
                if (!TOKEN_SUCC(result))
                        result = token_oct_to_int(c);
                else 
                        result = result * TOKEN_OCTAL_BASE 
                                + token_oct_to_int(c);

                if (n_chr > TOKEN_OCTAL_ESC_LEN)
                        break;
        }
        if (TOKEN_SUCC(result))
                *pos += n_chr + 1;
        return result;
}

static int64_t token_hex_esc_value(const char *value, size_t *pos)
{
        int64_t result = TOKEN_ESCAPE_UNMATCH;
        size_t n_chr = 1;
        char c;

        if (value[*pos + n_chr] == 'x') {
                result = TOKEN_ERROR;
                for ( ; token_is_hex(c = value[*pos + n_chr + 1]); n_chr++) {
                        if (!TOKEN_SUCC(result))
                                result = token_hex_to_int(c);
                        else 
                                result = result * TOKEN_HEX_BASE 
                                        + token_hex_to_int(c);

                        if (n_chr > TOKEN_HEX_ESC_LEN)
                                MC_LOG(MC_WARN, "max length exceeded");
                }
        }
        if (TOKEN_SUCC(result))
                *pos += n_chr + 1;
        return result;
}

static inline 
_Bool token_univ_chrname_valid(int64_t value)
{
        return !((value < 0xA0 && value != 0x24 && value != 0x40 
        && value != 0x60) || (value >= 0xD800 && value <= 0xDFFF));
}

/* We encode code point to a maximum 4 8 bit code units,
 * which are placed in result, such as the last present unit
 * will be in least significant byte */
int64_t token_utf8_pt_code_units(int64_t point)
{
        int64_t result;
        if (point < 0xA0) {
                result = point;
        } else if (point <= 0x07FF) {
                result = (0xC0 | (point >> 6)) << 8;
                result |= 0x80 | (point & 0x3F);
        } else if (point >= 0xD800 && point <= 0xDFFF) {
                result = TOKEN_ERROR;
        } else if (point <= 0xFFFF) {
                result = (0xE0 | (point >> 12)) << 16;
                result |= (0x80 | ((point >> 6) & 0x3F)) << 8;
                result |= 0x80 | (point & 0x3F);
        } else {
                /* you will have invalid result if pt > 10FFFF */
                result = (0xF0 | (point >> 18)) << 24;
                result |= (0x80 | ((point >> 12) & 0x3F)) << 16;
                result |= (0x80 | ((point >> 6) & 0x3F)) << 8;
                result |= 0x80 | (point & 0x3F);
                if (point > 0x10FFFF)
                        MC_LOG(MC_WARN, "point value exceeds representable");
        }
        return result;
}

/* We encode code point to a maximum 2 16 bit code units,
 * which are placed in result, such as the last present unit
 * will be in least significant word */
int64_t token_utf16_pt_code_units(int64_t point)
{
        int64_t result;
        if (point < 0xA0) {
                result = point;
        } else if (point >= 0xD800 && point <= 0xDFFF) {
                result = TOKEN_ERROR;
        } else if (point <= 0xFFFF) {
                assert(point < 0xD800 || point > 0xDFFF);
                result = point;
        } else {
                /* you will have invalid result if pt > 10FFFF */
                result = (0xD800 | (point >> 10)) << 16;
                result |= 0xDC00 | (point & 0x3FF);
                if (point > 0x10FFFF)
                        MC_LOG(MC_WARN, "point value exceeds representable");
        }
        return result;
}

static int64_t token_chrname_esc_value(const char *value, size_t *pos)
{
        int64_t result = TOKEN_ESCAPE_UNMATCH;
        size_t n_chr = 1;
        const int hex_quad = 4;
        int nhex;
        char c;

        if (toupper(value[*pos + n_chr]) == 'U') {
                result = TOKEN_ERROR;
                nhex = hex_quad + (value[*pos + n_chr] == 'U') * hex_quad;
                for (int i = 0; i < nhex; i++, n_chr++) {
                        if (!token_is_hex(c = value[*pos + n_chr + 1])) {
                                result = TOKEN_ERROR;
                                break;
                        }
                        if (result == TOKEN_ERROR)
                                result = token_hex_to_int(c);
                        else 
                                result = result * TOKEN_HEX_BASE 
                                        + token_hex_to_int(c);
                }
        }
        if (TOKEN_SUCC(result))
                *pos += n_chr + 1;
        return result;
}

static wchar_t token_char_to_wchar(int64_t value)
{
        wchar_t wres;
        char esc_temp = (char)value;
        assert(value <= UCHAR_MAX);
        if (mbtowc(&wres, &esc_temp, 1) == -1) {
                wres = TOKEN_ERROR;
                MC_LOG(MC_WARN, "char to wchar convertation failed");
        }
        return wres;
}

static size_t token_esc_val_to_wchars(_IN int64_t esc_val, 
                                      _OUT wchar_t *result)
{
        size_t n_sym;
        const int chr_base = WCHAR_MAX + 1;
        for (n_sym = 0; n_sym < TOKEN_ESC_VAL_MAX_LEN && esc_val != 0; n_sym++) {
                result[n_sym] = esc_val % chr_base;
                esc_val >>= sizeof(wchar_t) * CHAR_BIT;
        }
        if (esc_val != 0)
                MC_LOG(MC_WARN, "value %lld not fully written", esc_val);
        return n_sym;
}

static void reverse_bytes(char *bytes, size_t len)
{
        for (size_t i = 0; i < len / 2; i++) {
                char temp = bytes[i];
                bytes[i] = bytes[len - i - 1];
                bytes[len - i - 1] = temp;
        }
}

static size_t token_esc_val_to_chars(_IN int64_t esc_val, 
                                     _OUT char *result)
{
        size_t n_sym;
        const int chr_base = UCHAR_MAX + 1;
        for (n_sym = 0; n_sym < TOKEN_ESC_VAL_MAX_LEN && esc_val != 0; n_sym++) {
                result[n_sym] = esc_val % chr_base;
                esc_val >>= sizeof(char) * CHAR_BIT;
        }
        reverse_bytes(result, n_sym);
        if (esc_val != 0)
                MC_LOG(MC_WARN, "value %lld not fully written", esc_val);
        return n_sym;
}

/* try to get value of one of 4 possible escape sequences */
static int64_t token_escape_value(_IN const char *value,
                                  _IN _Bool is_wstr,
                                  _INOUT size_t *pos)
{
        int64_t esc_val;
        int64_t chr_base = (is_wstr ? WCHAR_MAX : UCHAR_MAX) + 1;
        esc_val = token_hex_esc_value(value, pos);
        if (TOKEN_SUCC(esc_val) || esc_val == TOKEN_ERROR) {
                if (TOKEN_SUCC(esc_val))
                        esc_val %= chr_base;
                return esc_val;
        }

        esc_val = token_octal_esc_value(value, pos);
        if (TOKEN_SUCC(esc_val) || esc_val == TOKEN_ERROR) {
                if (TOKEN_SUCC(esc_val))
                        esc_val %= chr_base;
                return esc_val;
        }

        esc_val = token_simple_esc_value(value, pos);
        if (TOKEN_SUCC(esc_val) && is_wstr)
                esc_val = token_char_to_wchar(esc_val);
        if (TOKEN_SUCC(esc_val) || esc_val == TOKEN_ERROR)
                return esc_val;

        esc_val = token_chrname_esc_value(value, pos);
        if (TOKEN_SUCC(esc_val)) {
                if (!token_univ_chrname_valid(esc_val))
                        esc_val = TOKEN_ERROR;
                else if (is_wstr)
                        esc_val = token_utf16_pt_code_units(esc_val);
                else
                        esc_val = token_utf8_pt_code_units(esc_val);
        }
        return esc_val;
}

/* @return wchar_t symbols count, which being returned in @result,
 * TOKEN_ESCAPE_UNMATCH in case there is not esc on current pos 
 * TOKEN_ERROR - error detected */
static int token_wescape_sequence(_IN struct pp_token *tok, 
                                  _INOUT size_t *pos, 
                                  _OUT wchar_t *result)
{
        size_t status = *pos;
        int64_t esc_val;

        if (tok->value[*pos] != '\\')
                return TOKEN_ESCAPE_UNMATCH;

        esc_val = token_escape_value(tok->value, true, pos);

        if (TOKEN_SUCC(esc_val)) {
                status = token_esc_val_to_wchars(esc_val, result);
        } else {
                *pos = status;
                status = esc_val;
        }
        return status;
}

/* @return char symbols count, which being returned in @result,
 * TOKEN_ESCAPE_UNMATCH in case there is not esc on current pos 
 * TOKEN_ERROR - error detected */
static int token_escape_sequence(_IN struct pp_token *tok, 
                                 _INOUT size_t *pos, 
                                 _OUT char *result)
{
        size_t status = *pos;
        int64_t esc_val;

        if (tok->value[*pos] != '\\')
                return TOKEN_ESCAPE_UNMATCH;

        esc_val = token_escape_value(tok->value, false, pos);

        if (TOKEN_SUCC(esc_val)) {
                status = token_esc_val_to_chars(esc_val, result);
        } else {
                *pos = status;
                status = esc_val;
        }
        return status;
}

static inline 
size_t token_strlit_start(struct pp_token *tok)
{
        if (tok->value[0] == 'L')
                return WSTRLIT_START_OFFSET;
        else
                return STRLIT_START_OFFSET;
}

static inline
_Bool token_is_space(struct pp_token *tok)
{
        return (tok->type == pp_other && 
                pp_token_valcmp(tok, "\n") == 0);

}

static struct token*
token_wchar_strlit(struct convert_context *ctx, size_t length)
{
        size_t wstr_len = length + 1;
        size_t raw_len = wstr_len * sizeof(wchar_t);
        wchar_t value[TOKEN_ESC_VAL_MAX_LEN];
        struct token *tok = token_create(convert_pos(ctx));
        struct pp_token *pos;
        size_t buf_pos = 0;
        _Bool not_last = true;

        /* TODO: estimate maximum possible size, to not to use realloc */
        wchar_t *buffer = calloc(raw_len, sizeof(wchar_t));
        tok->type = tok_strlit;

        size_t val_pos = token_strlit_start(convert_pos(ctx));
        while (buf_pos < wstr_len && not_last) {
                pos = convert_pos(ctx);
                if (pos->type != pp_str_lit) {
                        if (!token_is_space(convert_pos(ctx)))
                                break;
                        if ((not_last = convert_advance(ctx)))
                                val_pos = token_strlit_start(convert_pos(ctx));
                        continue;
                }
                
                int offset = token_wescape_sequence(pos, &val_pos, value);
                if (offset == TOKEN_ERROR) {
                        convert_error(ctx, "non-ISO-standart escape sequence");
                        goto fail;
                } else if (offset == TOKEN_ESCAPE_UNMATCH) {
                        offset = 1;
                        value[0] = token_char_to_wchar(pos->value[val_pos++]);
                }

                if (buf_pos + offset >= wstr_len) {
                        wstr_len = buf_pos + offset + 1;
                        raw_len = wstr_len * sizeof(wchar_t);
                        buffer = realloc(buffer, raw_len);
                }
                memcpy(&buffer[buf_pos], value, offset * sizeof(wchar_t));
                buf_pos += offset;

                if (val_pos >= pos->length - 1) {
                        not_last = convert_advance(ctx);
                        val_pos = token_strlit_start(convert_pos(ctx));
                }
        }
        tok->value.var_raw.value = (char *)buffer;
        tok->value.var_raw.length = (buf_pos + 1) * sizeof(wchar_t);
        return tok;

fail:
        token_destroy(tok);
        return NULL;
}

static struct token*
token_char_strlit(struct convert_context *ctx, size_t length)
{
        char value[TOKEN_ESC_VAL_MAX_LEN];
        size_t raw_len = (length + 1) * sizeof(char);
        struct token *tok = token_create(convert_pos(ctx));
        struct pp_token *pos;
        size_t val_pos = STRLIT_START_OFFSET;
        size_t buf_pos = 0;
        _Bool not_last = true;

        /* TODO: estimate maximum possible size, to not to use realloc */
        char *buffer = calloc(raw_len, sizeof(char));
        tok->type = tok_strlit;

        while (buf_pos < raw_len && not_last) {
                pos = convert_pos(ctx);
                if (pos->type != pp_str_lit) {
                        /* we touch next valuable tok, so we restore */
                        if (!token_is_space(pos)) {
                                convert_retreat(ctx);
                                break;
                        }
                        not_last = convert_advance(ctx);
                        val_pos = STRLIT_START_OFFSET;
                        continue;
                }
                int offset = token_escape_sequence(pos, &val_pos, value);
                if (offset == TOKEN_ERROR) {
                        convert_error(ctx, "invalid escape sequence");
                        goto fail;
                } else if (offset == TOKEN_ESCAPE_UNMATCH) {
                        offset = 1;
                        value[0] = pos->value[val_pos++];
                }

                if (buf_pos + offset >= raw_len) {
                        raw_len = buf_pos + offset + 1;
                        buffer = realloc(buffer, raw_len);
                }
                memcpy(&buffer[buf_pos], value, offset * sizeof(char));
                buf_pos += offset;

                if (val_pos >= pos->length - 1) {
                        convert_advance(ctx);
                        val_pos = STRLIT_START_OFFSET;
                }
        }
        tok->value.var_raw.value = buffer;
        tok->value.var_raw.is_alloc = true;
        tok->value.var_raw.length = (buf_pos + 1) * sizeof(char);
        return tok;

fail:
        token_destroy(tok);
        return NULL;
}

/* In translation phase 6, the multibyte character sequences 
 * specified by any sequence of adjacent character and wide 
 * string literal tokens are concatenated into a single multibyte
 * character sequence. */
static struct token*
token_str_literal(struct convert_context *ctx)
{
        struct token *tok;
        struct pp_token* pos = convert_pos(ctx);
        struct pp_token* first = pos;
        _Bool is_wide = false;
        size_t length = 0;

        do {
                pos = convert_pos(ctx);
                _Bool is_pos_wstr = token_is_wstr(pos);
                /* If any of the tokens are wide string literal tokens,
                 * the resulting multibyte character sequence is treated 
                 * as a wide string literal; otherwise, it is treated as 
                 * a character string literal. */
                if (is_pos_wstr)
                        is_wide = true;
                if (token_is_space(pos))
                        continue;
                else if (pos->type != pp_str_lit)
                        break;

                if (is_pos_wstr) {
                        assert(pos->length >= TOKEN_WSTRL_MIN_LEN);
                        length += pos->length - TOKEN_WSTRL_MIN_LEN;
                } else {
                        assert(pos->length >= TOKEN_STRL_MIN_LEN);
                        length += pos->length - TOKEN_STRL_MIN_LEN;
                }
        } while (convert_advance(ctx) && convert_continue(ctx));
        convert_setpos(ctx, first);
        if (is_wide)
                tok = token_wchar_strlit(ctx, length);
        else
                tok = token_char_strlit(ctx, length);
        return tok;
}

const char *token_punctuators[] = {
        "%:%:", "...", "<<=", ">>=",

        "->", "++", "--", "<<", ">>",
        "<=", ">=", "==", "!=", "&&",
        "||", "*=", "/=", "%=", "+=",
        "-=", "&=", "^=", "|=", "##",
        "<:", ":>", "<%", "%>", "%:",
        
        ".", "!", "-", "+", "*", "&",
        "<", ">", "^", "|", ":", "=",
        "#", ",", ";", "?", "[", "]",
        "(", ")", "{", "}", "~", "/",
        "%"
};

struct trie_node token_punc_trie;

static enum punc_type token_punctuator_type(struct pp_token *punct)
{
        int punc_num = trie_search(&token_punc_trie, punct->value, 
                punct->length);
        if (punc_num != punc_invalid && pp_token_valcmp(punct, 
                token_punctuators[punc_num]))
                punc_num = punc_invalid;
        return punc_num;
}

static struct token*
token_punctuator(struct convert_context *ctx)
{
        struct token *tok;
        struct pp_token *pos = convert_pos(ctx);
        enum punc_type type = token_punctuator_type(pos);
        assert(type != punc_invalid);
        tok = token_create(pos);
        tok->type = tok_punctuator;
        tok->value.var_punc = type;
        return tok;
}

static void
token_value_raw_realloc(_INOUT struct token *tok, size_t new_size)
{
        size_t buffer_size = max(new_size, tok->value.var_raw.length);
        if (tok->value.var_raw.is_alloc == false) {
                char *old_val = tok->value.var_raw.value;
                tok->value.var_raw.value = calloc(buffer_size, sizeof(char *));
                memcpy(tok->value.var_raw.value, old_val, buffer_size);
                tok->value.var_raw.is_alloc = true;
        } else {
                tok->value.var_raw.value = realloc(tok->value.var_raw.value, 
                        buffer_size);
        }
        tok->value.var_raw.length = new_size;
}

static _Bool
token_identifier_setvalue(_OUT struct token *tok, struct pp_token *id)
{
        char value[TOKEN_ESC_VAL_MAX_LEN];
        size_t offset = 0;
        int64_t esc_val;

        tok->type = tok_identifier;
        tok->value.var_raw.value = id->value;
        tok->value.var_raw.length = id->length;
        for (size_t i = 0, buffer_pos = 0; 
                i < id->length; i++) {
                if (id->value[i] != '\\') {
                        buffer_pos += 1;
                        continue;
                }
                esc_val = token_chrname_esc_value(id->value, &i);
                if (!TOKEN_SUCC(esc_val))
                        goto fail;

                if (buffer_pos == 0 && esc_val < INT_MAX && 
                        isdigit((int)esc_val))
                        goto fail;
                esc_val = token_utf8_pt_code_units(esc_val);
                if (!TOKEN_SUCC(esc_val))
                        goto fail;

                offset += token_esc_val_to_chars(esc_val, value);
                token_value_raw_realloc(tok, buffer_pos + offset);

                memcpy(&tok->value.var_raw.value[buffer_pos], value, offset);
                buffer_pos += offset;
        }
        return true;
fail: 
        return false;
}

static struct token*
token_identifier(struct convert_context *ctx)
{
        struct pp_token *pos = convert_pos(ctx);
        struct token *tok = token_create(pos);
        char *id_val = pos->value;

        int keyw_num = trie_search(&token_keyw_trie, id_val, pos->length);
        /* TODO: handle universal char name */
        if (keyw_num != keyw_invalid && pp_token_valcmp(pos, 
                token_keywords[keyw_num]) == 0) {
                tok->type = tok_keyword;
                tok->value.var_keyw = keyw_num;
        } else if (!token_identifier_setvalue(tok, pos)) {
                token_destroy(tok);
                tok = NULL;
        }
        return tok;
}

static void token_const_value_type(_IN struct pp_num_info info,
                                   _INOUT struct constant_value *val)
{ 
        static const int long_long_flags = const_long_long_int 
                | const_ulong_long_int;
        static const int long_flags = const_long_int | const_ulong_int 
                | long_long_flags;
        if (info.sign == pp_num_signed) {
                int64_t i_val = val->data.var_int;
                if (i_val <= INT_MAX && i_val >= INT_MIN)
                        val->type |= const_int;
                else if (i_val <= LONG_MAX && i_val >= LONG_MIN)
                        val->type |= const_long_int;
                else if (i_val <= LONG_LONG_MAX && i_val >= LONG_LONG_MIN)
                        val->type |= const_long_long_int;
        }
        
        if (info.sign == pp_num_unsigned || info.base != pp_num_decimal) {
                if (val->data.var_uint <= UINT_MAX)
                        val->type |= const_uint;
                else if (val->data.var_uint <= ULONG_MAX)
                        val->type |= const_ulong_int;
                else 
                        val->type |= const_ulong_long_int;
        }
        if (info.size == pp_num_size_long) {
                val->type &= long_flags;
                /* value is less the explictely specified size */
                if (val->type == 0) {
                        if (info.sign == pp_num_signed)
                                val->type = const_long_int;
                        else 
                                val->type = const_ulong_int;
                }
        } else if (info.size == pp_num_size_long_long) {
                val->type &= long_long_flags;
                if (val->type == 0) {
                        if (info.sign == pp_num_signed)
                                val->type = const_long_long_int;
                        else 
                                val->type = const_ulong_long_int;
                }
        }
        int bit_n = 0;
        while (!(val->type & 1)) {
                val->type >>= 1;
                bit_n += 1;
        }
        val->type = (1 << bit_n);
}

static _Bool token_parse_signed_int(_IN const char *restrict buffer,
                                    _IN struct pp_num_info info,
                                    _OUT struct constant_value *val)
{
        int nret = sscanf(buffer, "%lli", &val->data.var_int);
        if (nret != 1)
                return false;
        token_const_value_type(info, val);
        return true;
}

static _Bool token_parse_unsigned_int(_IN const char *restrict buffer,
                                      _IN struct pp_num_info info,
                                      _OUT struct constant_value *val)
{
        int nret;
        switch (info.base) {
                case pp_num_octal:
                        nret = sscanf(buffer, "%llo", &val->data.var_uint);
                        break;
                case pp_num_decimal:
                        nret = sscanf(buffer, "%llu", &val->data.var_uint);
                        break;
                case pp_num_hex:
                        nret = sscanf(buffer, "%llx", &val->data.var_uint);
                        break;
        }
        if (nret != 1)
                return false;
        token_const_value_type(info, val);
        return true;
}

static _Bool token_parse_float(_IN const char *restrict buffer,
                               _IN struct pp_num_info info,
                               _OUT struct constant_value *val)
{
        int nret = sscanf(buffer, "%LF", &val->data.var_long_double);
        if (nret != 1)
                return false;
        if (info.size == pp_num_size_float)
                val->type = const_float;
        else if (info.size == pp_num_size_long)
                val->type = const_long_double;
        else if (info.size == pp_num_size_no)
                val->type = const_double;
        else /* long long is invalid here */
                return false; 
        return true; 
}

static struct token*
token_number(struct convert_context *ctx)
{
        struct pp_token *pos = convert_pos(ctx);
        struct token *tok = token_create(pos);
        struct constant_value *val_const = &tok->value.var_const;
        char *buffer = calloc(pos->length + 1, sizeof(char));
        memcpy(buffer, pos->value, pos->length);
        _Bool status = false;

        tok->type = tok_constant;
        struct pp_num_info info = pp_token_num_parse(pos);
        if (info.type == pp_num_integer) {
                if (info.sign == pp_num_signed)
                        status = token_parse_signed_int(buffer, info, 
                                val_const);
                else
                        status = token_parse_unsigned_int(buffer, info, 
                                val_const);
        } else {
                status = token_parse_float(buffer, info, val_const);
        }
        if (!status) {
                token_destroy(tok);
                tok = NULL;
        }
        free(buffer);
        return tok;
}

static _Bool token_wchr_literal_value(struct pp_token* tok, int64_t *lit_val)
{
        wchar_t esc_val[TOKEN_ESC_VAL_MAX_LEN];
        size_t val_pos = WSTRLIT_START_OFFSET;
        int64_t const_val = 0;
        int char_bsize = CHAR_BIT * sizeof(wchar_t);

        while (val_pos < tok->length - 1) {
                int offset = token_wescape_sequence(tok, &val_pos, esc_val);
                if (offset == TOKEN_ESCAPE_UNMATCH) {
                        const_val <<= char_bsize;
                        char char_val = tok->value[val_pos++];
                        wchar_t wchar_val = token_char_to_wchar(char_val);
                        const_val |= wchar_val;
                } else if (TOKEN_SUCC(offset)) {
                        for (int esc_pos = 0; esc_pos < offset; esc_pos++) {
                                const_val <<= char_bsize;
                                const_val |= esc_val[esc_pos];
                        }
                } else {
                        return false;
                }
        }
        *lit_val = const_val;
        return true;
}

static _Bool token_chr_literal_value(struct pp_token* tok, int64_t *lit_val)
{
        char esc_val[TOKEN_ESC_VAL_MAX_LEN];
        size_t val_pos = STRLIT_START_OFFSET;
        int64_t const_val = 0;
        int char_bsize = CHAR_BIT * sizeof(char);

        while (val_pos < tok->length - 1) {
                int offset = token_escape_sequence(tok, &val_pos, esc_val);
                if (offset == TOKEN_ESCAPE_UNMATCH) {
                        const_val <<= char_bsize;
                        const_val |= tok->value[val_pos++];
                } else if (TOKEN_SUCC(offset)) {
                        for (int esc_pos = 0; esc_pos < offset; esc_pos++) {
                                const_val <<= char_bsize;
                                const_val |= esc_val[esc_pos];
                        }
                } else {
                        return false;
                }
        }
        *lit_val = const_val;
        return true;
}

static struct token*
token_chr_literal(struct convert_context *ctx)
{
        struct pp_token* tok = convert_pos(ctx);
        struct token *const_tok = token_create(tok);
        int64_t const_val;
        _Bool status;

        const_tok->type = tok_constant;
        if (token_is_wstr(tok)) {
                status = token_wchr_literal_value(tok, &const_val);
                const_tok->value.var_const.type = const_wchar_t;
        } else {
                status = token_chr_literal_value(tok, &const_val);
                const_tok->value.var_const.type = const_char;
        }
        if (!status) {
                convert_error(ctx, "invalid escape sequence");
                goto fail;
        }

        const_tok->value.var_const.data.var_int = const_val;
        return const_tok;
fail:
        token_destroy(const_tok);
        return NULL;
}

struct token* token_convert_next(struct convert_context *ctx)
{
        struct token* new_tok = NULL;
        switch (convert_pos(ctx)->type) {
                case pp_id:
                        new_tok = token_identifier(ctx);
                        if (new_tok == NULL)
                                convert_error(ctx, "invalid identifier");
                        break;
                case pp_number:
                        new_tok = token_number(ctx);
                        if (new_tok == NULL)
                                convert_error(ctx, "invalid number");
                        break;
                case pp_chr_const:
                        new_tok = token_chr_literal(ctx);
                        if (new_tok == NULL)
                                convert_error(ctx, "invalid number");
                        break;
                case pp_str_lit:
                        new_tok = token_str_literal(ctx);
                        if (new_tok == NULL)
                                convert_error(ctx, "invalid string literal");
                        break;
                case pp_punct:
                        new_tok = token_punctuator(ctx);
                        if (new_tok == NULL)
                                convert_error(ctx, "invalid punctuator");
                        break;
                case pp_other:
                        if (pp_token_valcmp(convert_pos(ctx), "\n") != 0)
                                MC_LOG(MC_CRIT, "Should be new-ln only");
                        break;
                default:
                        MC_LOG(MC_CRIT, "unexpected token");
                        break;
        }
        return new_tok;
}

void token_global_init()
{
        assert(keyw_invalid < TRIE_ALPHABET_SIZE);
        trie_init(&token_keyw_trie, keyw_invalid);
        for (size_t n_kw = 0; n_kw < ARRAY_SIZE(token_keywords); n_kw++) {
                const char *keyw = token_keywords[n_kw];
                trie_insert(&token_keyw_trie, keyw, n_kw);
        }

        trie_init(&token_punc_trie, punc_invalid);
        for (size_t n_p = 0; n_p < ARRAY_SIZE(token_punctuators); n_p++) {
                const char *punc = token_punctuators[n_p];
                trie_insert(&token_punc_trie, punc, n_p);
        }
}

void token_global_free()
{
        trie_free(&token_keyw_trie);
        trie_free(&token_punc_trie);
}

void token_print(struct token *tok)
{
        pp_print_token_line(tok->first);
}

#ifdef DEBUG

static const char *token_type_strs[] = {
        [tok_keyword] = "keyword",
        [tok_identifier] = "identifier",
        [tok_constant] = "constant",
        [tok_strlit] = "string literal",
        [tok_punctuator] = "punctuator",
};

static void token_print_constant(struct constant_value c_val)
{
        if (token_const_is_float(c_val.type))
                printf("%Lf", c_val.data.var_long_double);
        else if (token_const_is_integer(c_val.type))
                printf("0x%llx", c_val.data.var_uint);
        else if (token_const_is_char(c_val.type)) {
                if (c_val.type == const_wchar_t)
                        printf("%lc", (wchar_t)c_val.data.var_int);
                else
                        printf("%c", (char)c_val.data.var_int);
        } else
                MC_LOG(MC_CRIT, "unexpected value type");
}

void token_print_content(struct token *tok)
{
        union token_value t_val = tok->value; 
        MC_LOG(MC_DEBUG, "token %p", (void *)tok);
        printf("%s [", token_type_strs[tok->type]);

        switch (tok->type) {
                case tok_keyword:
                {
                        printf(token_keywords[t_val.var_keyw]);
                        break;
                }
                case tok_identifier:
                case tok_strlit:    
                {
                        for (file_size_t i = 0; i < t_val.var_raw.length; i++)
                                putchar(t_val.var_raw.value[i]);
                        break;
                }
                case tok_constant:
                {
                        token_print_constant(t_val.var_const);
                        break;

                }
                case tok_punctuator:
                {
                        printf(token_punctuators[t_val.var_punc]);
                        break;
                }
                default:
                {
                        MC_LOG(MC_CRIT, "unexpected token type");
                        break;
                }
                        
        }
        puts("]");
}

#endif /* DEBUG */