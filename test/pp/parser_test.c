#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <pp.h>
#include <test_common.h>


static void 
pp_parser_test_init(struct pp_parser *parser, struct pp_lexer *lex, 
        struct filesys *fs, const char *t_text)
{
        fs_init(fs);
        fs_add_local(fs, "./mc/pp/");
        fs_add_local(fs, "./");

        FILE *t_file = fopen(TFILE_NAME, "w");
        fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        fclose(t_file);

        struct fs_file *src = fs_get_local(fs, TFILE_NAME);
        assert(src != NULL);

        pp_lexer_init(lex, src);
        struct pp_token *token = NULL;
        while ((token = pp_lexer_get_token(lex)))
                pp_lexer_add_token(lex, token);
        assert(pp_lexer_noerror(lex));

        pp_parser_init(parser, lex->first);

}

static void pp_parser_test_free(struct pp_lexer *lex, struct filesys *fs)
{
        pp_lexer_free(lex);
        fs_free(fs);
}

TEST_CASE(pp_parser, multiline_macro)
{
        struct pp_parser parser = {0};
        struct pp_lexer lex = {0};
        struct filesys fs = {0};
        const char *t_text = "#ifdef _LIB_ \n"
                             "printf(\"error\\n\");\n"
                             "exit(1);\n"
                             "#endif\n"
                             "#include <mylib.cc>\n";
        pp_parser_test_init(&parser, &lex, &fs, t_text);
        
        struct pp_node *file = pp_node_create(pp_file);

        struct pp_node *node = pp_parser_fetch_node(&parser);
        ASSERT_NE(node, NULL)
        ASSERT_EQ(node->type, pp_if_group);
        pp_node_file_insert(file, node);
        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_leaf)
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "#"), 0);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "ifdef"), 0);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "_LIB_"), 0);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf); 
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "\n"), 0);
        
        node = pp_parser_fetch_node(&parser);
        ASSERT_NE(node, NULL);
        ASSERT_EQ(node->type, pp_text_line);
        pp_node_file_insert(file, node);

        node = pp_parser_fetch_node(&parser);
        ASSERT_NE(node, NULL);
        ASSERT_EQ(node->type, pp_text_line);
        pp_node_file_insert(file, node);

        node = pp_parser_fetch_node(&parser);
        ASSERT_NE(node, NULL);
        ASSERT_EQ(node->type, pp_endif_line);
        pp_node_file_insert(file, node);

        node = pp_parser_fetch_node(&parser);
        ASSERT_NE(node, NULL); 
        ASSERT_EQ(node->type, pp_control_line);
        pp_node_file_insert(file, node);
        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "#"), 0);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf); 
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "include"), 0);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_tokens);
        ASSERT_EQ(node->first_descendant->leaf_tok->type, pp_hdr_name);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "\n"), 0);

        pp_node_destroy(file);
        pp_parser_test_free(&lex, &fs);
}

TEST_CASE(pp_parser, func_def)
{
        struct pp_parser parser = {0};
        struct pp_lexer lex = {0};
        struct filesys fs = {0};
        const char *t_text = "pp_parser_get_pos(struct pp_parser *parser)\n"
                             "{\n"
                             "return parser->pos;\n"
                             "}";

        pp_parser_test_init(&parser, &lex, &fs, t_text);
        for (int i = 0; i < 4; i++) {
                struct pp_node *node = pp_parser_fetch_node(&parser);
                ASSERT_NE(node, NULL);
                pp_node_destroy(node);
        }
        
        pp_parser_test_free(&lex, &fs);
}

TEST_CASE(pp_parser, macro_if_elif)
{
        struct pp_parser parser = {0};
        struct pp_lexer lex = {0};
        struct filesys fs = {0};
        const char *t_text = "#if DEBUG==1 \n"
                             "#elif DEBUG&&1 \n";

        pp_parser_test_init(&parser, &lex, &fs, t_text);
        struct pp_node *file = pp_node_create(pp_file);

        struct pp_node *node = pp_parser_fetch_node(&parser);
        ASSERT_NE(node, NULL);
        ASSERT_EQ(node->type, pp_if_group);
        pp_node_file_insert(file, node);
        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "#"), 0);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "if"), 0);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_const_expr);
        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "\n"), 0);
        
        node = pp_parser_fetch_node(&parser);
        ASSERT_NE(node, NULL);
        ASSERT_EQ(node->type, pp_elif_group);
        pp_node_file_insert(file, node);
        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "#"), 0);

        node = list_next(node);
        ASSERT_EQ(node->type, pp_leaf);
        ASSERT_EQ(pp_token_valcmp(node->leaf_tok, "elif"), 0);

        node = list_next(node);
        ASSERT_EQ(node->type, pp_const_expr);

        pp_node_destroy(file);
        pp_parser_test_free(&lex, &fs);
}

void run_pp_parser()
{
        mc_init(0, NULL);
        TEST_RUN(pp_parser, multiline_macro);
        TEST_RUN(pp_parser, macro_if_elif);
        TEST_RUN(pp_parser, func_def);
        mc_free();
}