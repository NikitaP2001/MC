#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <stdbool.h>

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
        if (tok->type == tok_strlit && tok->value.var_raw.value != NULL)
                free(tok->value.var_raw.value);
        free(tok);
}

/* order should match enum token_type */
static const char* keywords[] = {
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

static uint8_t keyw_table[PEARSON_TABLE_SIZE];

static struct token*
token_identifier(struct pp_token *unit_pos)
{
        struct token *tok = token_create(unit_pos);
        char *id_val = unit_pos->value;
        if (tok == NULL)
                return NULL;

        uint8_t hash = pearson_hash(id_val, unit_pos->length);
        int keyw_num = keyw_table[hash];
        /* TODO: handle universal char name */
        if (keyw_num != keyw_invalid && pp_token_valcmp(unit_pos, 
                keywords[keyw_num]) == 0) {
                tok->type = tok_keyword;
                tok->value.var_keyw = keyw_num;
        } else {
                tok->type = tok_identifier;
                tok->value.var_raw.value = unit_pos->value;
        }
        return tok;
}

static inline _Bool token_is_wstr(struct pp_token *strlit)
{
        return strlit->value[0] == 'L';
}

const char token_simple_esc_chars[] = "'\"?\\abfnrtv";
const char token_simple_esc_vals[] = "\'\"\?\a\b\f\n\r\t\v";

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
        if (point <= 0x007F) {
                if (point == 0x24 || point == 0x40 || point == 0x60)
                        result = point;
                else
                        result = TOKEN_ERROR;
        } else if (point < 0xA0) {
                point = TOKEN_ERROR;
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
        if (point <= 0x007F) {
                if (point == 0x24 || point == 0x40 || point == 0x60)
                        result = point;
                else
                        result = TOKEN_ERROR;
        } else if (point < 0xA0 || (point >= 0xD800 && point <= 0xDFFF)) {
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
token_wchar_strlit(struct pp_token *pos, size_t length)
{
        size_t wstr_len = length + 1;
        size_t raw_len = wstr_len * sizeof(wchar_t);
        wchar_t value[TOKEN_ESC_VAL_MAX_LEN];
        struct token *tok = token_create(pos);
        size_t buf_pos = 0;

        /* TODO: estimate maximum possible size, to not to use realloc */
        wchar_t *buffer = calloc(raw_len, sizeof(wchar_t));
        tok->type = tok_strlit;

        size_t val_pos = token_strlit_start(pos);
        while (buf_pos < wstr_len && pos != NULL) {
                if (pos->type != pp_str_lit) {
                        if (!token_is_space(pos))
                                break;
                        pos = list_next(pos);
                        if (pos != NULL)
                                val_pos = token_strlit_start(pos);
                        continue;
                }
                
                int offset = token_wescape_sequence(pos, &val_pos, value);
                if (offset == TOKEN_ERROR) {
                        token_error(pos, "non-ISO-standart escape sequence");
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
                        pos = list_next(pos);
                        val_pos = token_strlit_start(pos);
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
token_char_strlit(struct pp_token *pos, size_t length)
{
        char value[TOKEN_ESC_VAL_MAX_LEN];
        size_t raw_len = (length + 1) * sizeof(char);
        struct token *tok = token_create(pos);
        size_t val_pos = STRLIT_START_OFFSET;
        size_t buf_pos = 0;

        /* TODO: estimate maximum possible size, to not to use realloc */
        char *buffer = calloc(raw_len, sizeof(char));
        tok->type = tok_strlit;

        while (buf_pos < raw_len && pos != NULL) {
                if (pos->type != pp_str_lit) {
                        if (!token_is_space(pos))
                                break;
                        pos = list_next(pos);
                        val_pos = STRLIT_START_OFFSET;
                        continue;
                }
                int offset = token_escape_sequence(pos, &val_pos, value);
                if (offset == TOKEN_ERROR) {
                        token_error(pos, "invalid escape sequence");
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
                        pos = list_next(pos);
                        val_pos = STRLIT_START_OFFSET;
                }
        }
        tok->value.var_raw.value = buffer;
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
token_str_literal(struct pp_token **first)
{
        struct token *tok;
        struct pp_token* pos;
        struct pp_token* last = *first;
        _Bool is_wide = false;
        size_t length = 0;

        LIST_FOREACH_ENTRY(*first) {
                pos = (struct pp_token*)entry;
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
                last = pos;
        }
        if (is_wide)
                tok = token_wchar_strlit(*first, length);
        else
                tok = token_char_strlit(*first, length);
        if (tok != NULL)
                *first = last; 
        return tok;
}

struct token *token_convert(struct pp_context *pp)
{
        struct token *result = NULL, *head = NULL;
        assert(mc_isinit());
        LIST_FOREACH_ENTRY(pp_node_leftmost_leaf(pp->root_file)) {
                struct pp_token* unit_pos = (struct pp_token*)entry;
                struct token* new_tok = NULL;
                switch (unit_pos->type) {
                        case pp_id:
                                new_tok = token_identifier(unit_pos);
                                break;
                        case pp_number:
                                break;
                        case pp_chr_const:
                                break;
                        case pp_str_lit:
                                new_tok = token_str_literal(&unit_pos);
                                break;
                        case pp_punct:
                                break;
                        case pp_other:
                                if (pp_token_valcmp(unit_pos, "\n") != 0)
                                        goto fail;
                                break;
                        default:
                                MC_LOG(MC_CRIT, "unexpected token");
                                goto fail;
                }
                if (new_tok != NULL) {
                        if (result == NULL)
                                head = result = new_tok;
                        else {
                                list_append(head, new_tok);
                                head = new_tok;
                        }
                }
                entry = (struct list_head *)unit_pos;
        }
        return result;
fail:
        list_destroy(result, (list_free)token_destroy);
        return NULL;
}

void token_init()
{
        assert(keyw_invalid < UINT8_MAX);
        memset(keyw_table, keyw_invalid, sizeof(keyw_table));
        for (size_t n_kw = 0; n_kw < ARRAY_SIZE(keywords); n_kw++) {
                const char *keyw = keywords[n_kw];
                uint8_t hash = pearson_hash(keyw, strlen(keyw));
                keyw_table[hash] = n_kw;
        }

}