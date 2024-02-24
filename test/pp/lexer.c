#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <pp.h>
#include <fs.h>
#include <test_common.h>

#define TFILE_NAME "file.cc"

static void 
check_next_token(struct pp_lexer *lex, enum pp_type type, const char *value)
{
        struct pp_token *token = pp_lexer_get_token(lex);
        assert(token->type == type && pp_token_valcmp(
                token, value) == 0);
        pp_lexer_add_token(lex, token);
}

static void pp_lexer_header_name()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "#include <h_dr/name.cc>\n";
        write_file(TFILE_NAME, t_text, LEN_OF(t_text));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        assert(pp_lexer_init(&lex, src));
        check_next_token(&lex, pp_punct, "#");
        check_next_token(&lex, pp_id, "include");
        check_next_token(&lex, pp_hdr_name, "<h_dr/name.cc>");
        check_next_token(&lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_header_name_multiline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "#include <file\n>\n";
        write_file(TFILE_NAME, t_text, LEN_OF(t_text));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);
        assert(pp_lexer_init(&lex, src));
        check_next_token(&lex, pp_punct, "#");
        check_next_token(&lex, pp_id, "include");
        check_next_token(&lex, pp_punct, "<");
        check_next_token(&lex, pp_id, "file");
        check_next_token(&lex, pp_other, "\n");
        check_next_token(&lex, pp_punct, ">");
        check_next_token(&lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_header_name_escline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "#include <h_dr/n\\\name.cc>\n";
        write_file(TFILE_NAME, t_text, LEN_OF(t_text));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        assert(pp_lexer_init(&lex, src));
        check_next_token(&lex, pp_punct, "#");
        check_next_token(&lex, pp_id, "include");
        check_next_token(&lex, pp_hdr_name, "<h_dr/name.cc>");
        check_next_token(&lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_str_lit_oneline()
{ 
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "\"lit 943\\123\" \"1 line\"\n"
        "\"2 line\"";
        write_file(TFILE_NAME, t_text, LEN_OF(t_text));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);
        assert(pp_lexer_init(&lex, src));
        check_next_token(&lex, pp_str_lit, "\"lit 943\\123\"");
        check_next_token(&lex, pp_str_lit, "\"1 line\"");
        check_next_token(&lex, pp_other, "\n");
        check_next_token(&lex, pp_str_lit, "\"2 line\"");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_str_lit_multline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "\"1 line\\\n2 line\"";
        write_file(TFILE_NAME, t_text, LEN_OF(t_text));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        assert(pp_lexer_init(&lex, src));
        check_next_token(&lex, pp_str_lit, "\"1 line2 line\"");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_comment_star_oneline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "/*begin*/#define /*abcde */ SOME_DEF 1233\n";
        write_file(TFILE_NAME, t_text, LEN_OF(t_text));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        assert(pp_lexer_init(&lex, src));
        check_next_token(&lex, pp_punct, "#");
        check_next_token(&lex, pp_id, "define");
        check_next_token(&lex, pp_id, "SOME_DEF");
        check_next_token(&lex, pp_number, "1233");
        check_next_token(&lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

void run_pp_lexer()
{
        puts("PP LEXER TESTS STARTED");
        pp_lexer_header_name();
        pp_lexer_header_name_multiline();
        pp_lexer_header_name_escline();
        pp_lexer_str_lit_oneline();
        pp_lexer_str_lit_multline();
        pp_lexer_comment_star_oneline();
        puts("PP LEXER TESTS FINISHED OK");
}