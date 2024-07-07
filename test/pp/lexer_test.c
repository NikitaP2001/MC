#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <pp.h>
#include <fs.h>
#include <test_common.h>

/* @ex_type - expected pp token type 
 * @value - char * value of token */ 
#define CHECK_NEXT_TOKEN(lex, ex_type, value)                      \
{                                                                  \
        struct pp_token *token = pp_lexer_get_token(&lex);         \
        EXPECT_EQ(token->type, ex_type);                           \
        EXPECT_EQ(pp_token_valcmp(token, value), 0);               \
        pp_lexer_add_token(&lex, token);                           \
}

TEST_CASE(lexer, header_name)
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        mc_init();
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "#include <h_dr/name.cc>\n";
        ASSERT_TRUE(write_file(TFILE_NAME, t_text, LEN_OF(t_text)));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        ASSERT_TRUE((pp_lexer_init(&lex, src)));
        CHECK_NEXT_TOKEN(lex, pp_punct, "#");
        CHECK_NEXT_TOKEN(lex, pp_id, "include");
        CHECK_NEXT_TOKEN(lex, pp_hdr_name, "<h_dr/name.cc>");
        CHECK_NEXT_TOKEN(lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

TEST_CASE(lexer, header_name_multiline)
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        mc_init();
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "#include <file\n>\n";
        ASSERT_TRUE(write_file(TFILE_NAME, t_text, LEN_OF(t_text)));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);
        ASSERT_TRUE(pp_lexer_init(&lex, src));
        CHECK_NEXT_TOKEN(lex, pp_punct, "#");
        CHECK_NEXT_TOKEN(lex, pp_id, "include");
        CHECK_NEXT_TOKEN(lex, pp_punct, "<");
        CHECK_NEXT_TOKEN(lex, pp_id, "file");
        CHECK_NEXT_TOKEN(lex, pp_other, "\n");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">");
        CHECK_NEXT_TOKEN(lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

TEST_CASE(lexer, header_name_escline)
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        mc_init();
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "#include <h_dr/n\\\name.cc>\n";
        ASSERT_TRUE(write_file(TFILE_NAME, t_text, LEN_OF(t_text)));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        ASSERT_TRUE((pp_lexer_init(&lex, src)));
        CHECK_NEXT_TOKEN(lex, pp_punct, "#");
        CHECK_NEXT_TOKEN(lex, pp_id, "include");
        CHECK_NEXT_TOKEN(lex, pp_hdr_name, "<h_dr/name.cc>");
        CHECK_NEXT_TOKEN(lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

TEST_CASE(lexer, str_lit_oneline)
{ 
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        mc_init();
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "\"lit 943\\123\" \"1 line\"\n"
        "\"2 line\"\"\\U00023456\"";
        ASSERT_TRUE(write_file(TFILE_NAME, t_text, LEN_OF(t_text)));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);
        ASSERT_TRUE(pp_lexer_init(&lex, src));
        CHECK_NEXT_TOKEN(lex, pp_str_lit, "\"lit 943\\123\"");
        CHECK_NEXT_TOKEN(lex, pp_str_lit, "\"1 line\"");
        CHECK_NEXT_TOKEN(lex, pp_other, "\n");
        CHECK_NEXT_TOKEN(lex, pp_str_lit, "\"2 line\"");
        CHECK_NEXT_TOKEN(lex, pp_str_lit, "\"\\U00023456\"");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

TEST_CASE(lexer, str_lit_multline)
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        mc_init();
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "\"1 line\\\n2 line\"";
        ASSERT_TRUE(write_file(TFILE_NAME, t_text, LEN_OF(t_text)));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        ASSERT_TRUE(pp_lexer_init(&lex, src));
        CHECK_NEXT_TOKEN(lex, pp_str_lit, "\"1 line2 line\"");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

TEST_CASE(lexer, comment_star_oneline)
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        mc_init();
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "/*begin*/#define /*abcde */ SOME_DEF 1233\n";
        ASSERT_TRUE(write_file(TFILE_NAME, t_text, LEN_OF(t_text)));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        ASSERT_TRUE(pp_lexer_init(&lex, src));
        CHECK_NEXT_TOKEN(lex, pp_punct, "#");
        CHECK_NEXT_TOKEN(lex, pp_id, "define");
        CHECK_NEXT_TOKEN(lex, pp_id, "SOME_DEF");
        CHECK_NEXT_TOKEN(lex, pp_number, "1233");
        CHECK_NEXT_TOKEN(lex, pp_other, "\n");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

TEST_CASE(lexer, punctuators)
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        mc_init();
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char t_text[] = "<= >= == != && || -> ++ -- << >> "
                "*= /= %= += -= <<= >>= &= ^= |= ## "
                "... %:%: <: :> <% %> "
                "[ ] ( ) { } . & * + - ~ ! / % < > ^ | ? : ; = , # "
                "<=> <!=> <===> ";
        ASSERT_TRUE(write_file(TFILE_NAME, t_text, LEN_OF(t_text)));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        ASSERT_TRUE(pp_lexer_init(&lex, src));
        CHECK_NEXT_TOKEN(lex, pp_punct, "<=");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "==");
        CHECK_NEXT_TOKEN(lex, pp_punct, "!=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "&&");
        CHECK_NEXT_TOKEN(lex, pp_punct, "||");
        CHECK_NEXT_TOKEN(lex, pp_punct, "->");
        CHECK_NEXT_TOKEN(lex, pp_punct, "++");
        CHECK_NEXT_TOKEN(lex, pp_punct, "--");
        CHECK_NEXT_TOKEN(lex, pp_punct, "<<");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">>");

        CHECK_NEXT_TOKEN(lex, pp_punct, "*=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "/=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "%=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "+=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "-=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "<<=");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">>=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "&=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "^=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "|=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "##");

        CHECK_NEXT_TOKEN(lex, pp_punct, "...");
        CHECK_NEXT_TOKEN(lex, pp_punct, "%:%:");

        CHECK_NEXT_TOKEN(lex, pp_punct, "<:");
        CHECK_NEXT_TOKEN(lex, pp_punct, ":>");
        CHECK_NEXT_TOKEN(lex, pp_punct, "<%");
        CHECK_NEXT_TOKEN(lex, pp_punct, "%>");

        CHECK_NEXT_TOKEN(lex, pp_punct, "[");
        CHECK_NEXT_TOKEN(lex, pp_punct, "]");
        CHECK_NEXT_TOKEN(lex, pp_punct, "(");
        CHECK_NEXT_TOKEN(lex, pp_punct, ")");
        CHECK_NEXT_TOKEN(lex, pp_punct, "{");
        CHECK_NEXT_TOKEN(lex, pp_punct, "}");
        CHECK_NEXT_TOKEN(lex, pp_punct, ".");
        CHECK_NEXT_TOKEN(lex, pp_punct, "&");
        CHECK_NEXT_TOKEN(lex, pp_punct, "*");
        CHECK_NEXT_TOKEN(lex, pp_punct, "+");
        CHECK_NEXT_TOKEN(lex, pp_punct, "-");
        CHECK_NEXT_TOKEN(lex, pp_punct, "~");
        CHECK_NEXT_TOKEN(lex, pp_punct, "!");
        CHECK_NEXT_TOKEN(lex, pp_punct, "/");
        CHECK_NEXT_TOKEN(lex, pp_punct, "%");
        CHECK_NEXT_TOKEN(lex, pp_punct, "<");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">");
        CHECK_NEXT_TOKEN(lex, pp_punct, "^");
        CHECK_NEXT_TOKEN(lex, pp_punct, "|");
        CHECK_NEXT_TOKEN(lex, pp_punct, "?");
        CHECK_NEXT_TOKEN(lex, pp_punct, ":");
        CHECK_NEXT_TOKEN(lex, pp_punct, ";");
        CHECK_NEXT_TOKEN(lex, pp_punct, "=");
        CHECK_NEXT_TOKEN(lex, pp_punct, ",");
        CHECK_NEXT_TOKEN(lex, pp_punct, "#");

        CHECK_NEXT_TOKEN(lex, pp_punct, "<=");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">");
        CHECK_NEXT_TOKEN(lex, pp_punct, "<");
        CHECK_NEXT_TOKEN(lex, pp_punct, "!=");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">");
        CHECK_NEXT_TOKEN(lex, pp_punct, "<=");
        CHECK_NEXT_TOKEN(lex, pp_punct, "==");
        CHECK_NEXT_TOKEN(lex, pp_punct, ">");

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

void run_pp_lexer()
{
        TEST_RUN(lexer, header_name);
        TEST_RUN(lexer, header_name_multiline);
        TEST_RUN(lexer, header_name_escline);
        TEST_RUN(lexer, str_lit_oneline);
        TEST_RUN(lexer, str_lit_multline);
        TEST_RUN(lexer, comment_star_oneline);
        TEST_RUN(lexer, punctuators);
}