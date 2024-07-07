#include <stdlib.h>
#include <string.h>

#include <pp.h>
#include <test_common.h>

TEST_CASE(pp_node, insert_macro)
{
        struct pp_parser parser = {0};
        struct pp_lexer lex = {0};
        struct filesys fs = {0};
        _Bool status = true;
        fs_init(&fs);
        fs_add_local(&fs, "./mc/pp/");
        fs_add_local(&fs, "./");
        const char t_text[] = "#if ! _LIB_ \n"
                             "  #error err msg \n"
                             "#elif DEF1&&DEF2\n"
                             "  status = 1\n"
                             "  exit(0);\n"
                             "#elif DEF3\n"
                             "  #if DEF4\n"
                             "    exit(1);\n"
                             "  #else\n"
                             "    exit(2);\n"
                             "  #endif\n"
                             "#endif\n"
                             "#include <lib.c>\n";
        write_file(TFILE_NAME, t_text, LEN_OF(t_text));
        ASSERT_EQ(strlen(t_text), LEN_OF(t_text));

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);
        ASSERT_NE(src, NULL);

        pp_lexer_init(&lex, src);
        struct pp_token *token = NULL;
        while ((token = pp_lexer_get_token(&lex)))
                pp_lexer_add_token(&lex, token);
        ASSERT_TRUE(pp_lexer_noerror(&lex));

        pp_parser_init(&parser, lex.first);
        struct pp_node *file = pp_node_create(pp_file);

        struct pp_node *node = NULL;
        while ((node = pp_parser_fetch_node(&parser))) {
                status = pp_node_file_insert(file, node);
                ASSERT_TRUE(status);
        }

        node = file->first_descendant;
        ASSERT_EQ(node->type, pp_group);
        
        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_group_part);

        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_if_section);
        
        node = node->first_descendant;
        struct pp_node *n_sect = node;
        ASSERT_EQ(node->type, pp_if_group);

        node = node->last_descendant;
        assert(node->type == pp_group);

        node = node->last_descendant;
        ASSERT_EQ(node->type, pp_group_part);

        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_control_line);

        n_sect = list_next(n_sect);
        ASSERT_EQ(n_sect->type, pp_elif_groups);

        node = n_sect->first_descendant;
        ASSERT_EQ(node->type, pp_elif_group);

        node = node->last_descendant;
        ASSERT_EQ(node->type, pp_group);

        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_group_part);
        ASSERT_EQ(node->first_descendant->type, pp_text_line);

        node = list_next(node);
        ASSERT_EQ(node->type, pp_group_part);
        ASSERT_EQ(node->first_descendant->type, pp_text_line);

        node = list_next(node);
        ASSERT_EQ(node, NULL);

        node = n_sect->first_descendant;
        node = list_next(node);

        ASSERT_EQ(node->type, pp_elif_group);

        node = node->last_descendant;
        ASSERT_EQ(node->type, pp_group);

        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_group_part);

        node = node->first_descendant;
        ASSERT_EQ(node->type, pp_if_section);

        node = file->first_descendant;
        ASSERT_EQ(node->type, pp_group);
        
        node = node->last_descendant;
        ASSERT_EQ(node->type, pp_group_part);

        node = node->last_descendant;
        ASSERT_EQ(node->type, pp_control_line);

        pp_node_destroy(file);
        pp_lexer_free(&lex);
        fs_free(&fs);
}


void run_pp_node()
{
        TEST_RUN(pp_node, insert_macro);
}