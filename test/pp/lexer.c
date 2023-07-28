#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <pp.h>
#include <fs.h>

#define TFILE_NAME "file.cc"

static void pp_lexer_header_name()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        size_t result = 0;
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char *t_text = "#include <h_dr/name.cc>\n";
        FILE *t_file = fopen(TFILE_NAME, "w");
        result = fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        assert(result != 0);
        fclose(t_file);

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        pp_lexer_init(&lex, src);
        struct pp_token *token = pp_lexer_get_token(&lex);
        assert(token->type == pp_punct && pp_token_valcmp(
                token, "#") == 0);
        pp_lexer_add_token(&lex, token);
        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_id && pp_token_valcmp(
                token, "include") == 0);
        pp_lexer_add_token(&lex, token);
        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_hdr_name && pp_token_valcmp(
                token, "<h_dr/name.cc>") == 0);
        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_other && pp_token_valcmp(
                token, "\n") == 0);
        pp_lexer_add_token(&lex, token);

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_header_name_multiline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        size_t result = 0;
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char *t_text = "#include <file\n>\n";
        FILE *t_file = fopen(TFILE_NAME, "w");
        result = fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        assert(result != 0);
        fclose(t_file);

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        pp_lexer_init(&lex, src);
        struct pp_token *token = pp_lexer_get_token(&lex);
        assert(token->type == pp_punct && pp_token_valcmp(
                token, "#") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_id && pp_token_valcmp(
                token, "include") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_punct && pp_token_valcmp(
                token, "<") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_id && pp_token_valcmp(
                token, "file") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_other && pp_token_valcmp(
                token, "\n") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_punct && pp_token_valcmp(
                token, ">") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_other && pp_token_valcmp(
                token, "\n") == 0);
        pp_lexer_add_token(&lex, token);

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_header_name_escline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        size_t result = 0;
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char *t_text = "#include <h_dr/n\\\name.cc>\n";
        FILE *t_file = fopen(TFILE_NAME, "w");
        result = fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        assert(result != 0);
        fclose(t_file);

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        pp_lexer_init(&lex, src);
        struct pp_token *token = pp_lexer_get_token(&lex);
        assert(token->type == pp_punct && pp_token_valcmp(
                token, "#") == 0);
        pp_lexer_add_token(&lex, token);
        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_id && pp_token_valcmp(
                token, "include") == 0);
        pp_lexer_add_token(&lex, token);
        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_hdr_name && pp_token_valcmp(
                token, "<h_dr/name.cc>") == 0);
        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_other && pp_token_valcmp(
                token, "\n") == 0);
        pp_lexer_add_token(&lex, token);

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_str_lit_oneline()
{ 
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        size_t result = 0;
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char *t_text = "\"lit 943\\123\" \"1 line\"\n"
        "\"2 line\"";
        FILE *t_file = fopen(TFILE_NAME, "w");
        result = fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        assert(result != 0);
        fclose(t_file);

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        pp_lexer_init(&lex, src);
        struct pp_token *token = pp_lexer_get_token(&lex);
        assert(token->type == pp_str_lit && pp_token_valcmp(
                token, "\"lit 943\\123\"") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_str_lit && pp_token_valcmp(
                token, "\"1 line\"") == 0);
        pp_lexer_add_token(&lex, token);
        
        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_other && pp_token_valcmp(
                token, "\n") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_str_lit && pp_token_valcmp(
                token, "\"2 line\"") == 0);
        pp_lexer_add_token(&lex, token);

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_str_lit_multline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        size_t result = 0;
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char *t_text = "\"1 line\\\n2 line\"";
        FILE *t_file = fopen(TFILE_NAME, "w");
        result = fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        assert(result != 0);
        fclose(t_file);

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        pp_lexer_init(&lex, src);
        struct pp_token *token = pp_lexer_get_token(&lex);
        assert(token->type == pp_str_lit && pp_token_valcmp(
                token, "\"1 line2 line\"") == 0);
        pp_lexer_add_token(&lex, token);

        pp_lexer_free(&lex);
        fs_free(&fs);
        remove(TFILE_NAME);
}

static void pp_lexer_comment_star_oneline()
{
        struct filesys fs = {0};
        struct pp_lexer lex = {0};
        size_t result = 0;
        fs_init(&fs);
        fs_add_local(&fs, "./");

        const char *t_text = "#define /*abcde */ SOME_DEF 1233\n";
        FILE *t_file = fopen(TFILE_NAME, "w");
        result = fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        assert(result != 0);
        fclose(t_file);

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);

        pp_lexer_init(&lex, src);
        struct pp_token *token = pp_lexer_get_token(&lex);
        assert(token->type == pp_punct && pp_token_valcmp(
                token, "#") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_id && pp_token_valcmp(
                token, "define")== 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_id && pp_token_valcmp(
                token, "SOME_DEF") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_number && pp_token_valcmp(
                token, "1233") == 0);
        pp_lexer_add_token(&lex, token);

        token = pp_lexer_get_token(&lex);
        assert(token->type == pp_other && pp_token_valcmp(
                token, "\n") == 0);
        pp_lexer_add_token(&lex, token);

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