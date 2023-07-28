#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <pp.h>

#define TFILE_NAME "test.cc"

static void pp_node_insert_macro()
{
        struct pp_parser parser = {0};
        struct pp_lexer lex = {0};
        struct filesys fs = {0};
        _Bool status = true;
        size_t result = 0;
        fs_init(&fs);
        fs_add_local(&fs, "./mc/pp/");
        fs_add_local(&fs, "./");
        const char *t_text = "#if ! _LIB_ \n"
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
        FILE *t_file = fopen(TFILE_NAME, "w");
        result = fwrite(t_text, sizeof(char), strlen(t_text), t_file);
        assert(result != 0);
        fclose(t_file);

        struct fs_file *src = fs_get_local(&fs, TFILE_NAME);
        assert(src != NULL);

        pp_lexer_init(&lex, src);
        struct pp_token *token = NULL;
        while ((token = pp_lexer_get_token(&lex)))
                pp_lexer_add_token(&lex, token);
        assert(pp_lexer_noerror(&lex));

        pp_parser_init(&parser, lex.first);
        struct pp_node *file = pp_node_create(pp_file);

        struct pp_node *node = NULL;
        while ((node = pp_parser_fetch_node(&parser))) {
                status = pp_node_file_insert(file, node);
                assert(status);
        }

        node = file->first_descendant;
        assert(node->type == pp_group);
        
        node = node->first_descendant;
        assert(node->type == pp_group_part);

        node = node->first_descendant;
        assert(node->type == pp_if_section);
        
        node = node->first_descendant;
        struct pp_node *n_sect = node;
        assert(node->type == pp_if_group);

        node = node->last_descendant;
        assert(node->type == pp_group);

        node = node->last_descendant;
        assert(node->type == pp_group_part);

        node = node->first_descendant;
        assert(node->type == pp_control_line);

        n_sect = list_next(n_sect);
        assert(n_sect->type == pp_elif_groups);

        node = n_sect->first_descendant;
        assert(node->type == pp_elif_group);

        node = node->last_descendant;
        assert(node->type == pp_group);

        node = node->first_descendant;
        assert(node->type == pp_group_part);
        assert(node->first_descendant->type == pp_text_line);

        node = list_next(node);
        assert(node->type == pp_group_part);
        assert(node->first_descendant->type == pp_text_line);

        node = list_next(node);
        assert(node == NULL);

        node = n_sect->first_descendant;
        node = list_next(node);

        assert(node->type == pp_elif_group);

        node = node->last_descendant;
        assert(node->type == pp_group);

        node = node->first_descendant;
        assert(node->type == pp_group_part);

        node = node->first_descendant;
        assert(node->type == pp_if_section);

        node = file->first_descendant;
        assert(node->type == pp_group);
        
        node = node->last_descendant;
        assert(node->type == pp_group_part);

        node = node->last_descendant;
        assert(node->type == pp_control_line);

        pp_node_destroy(file);
        pp_lexer_free(&lex);
        fs_free(&fs);
}


void run_pp_node()
{
        puts("PP NODE TESTS STARTED");
        pp_node_insert_macro();
        puts("PP NODE TESTS FINISHED OK");
}