#ifndef _COMMON_H_
#define _COMMON_H_
#include <parser/symtable.h>
#include <pp/token.h>
#include <parser.h>

struct parser_test_config {
        const char *file_name;
        const char *code_snippet;
};

struct parser_test_context {
        struct filesys fs;
        struct pp_context pp;
        struct convert_context c_ctx;
        struct parser parser;
        struct parser_context {
                struct token *curr;
                struct token *last_pull;
                struct token *last_error;
        } p_ctx;
        struct parser_test_config t_cfg;
};

void parser_test_setup_snippet(struct parser_test_context *t_ctx,
			       const char *code_snippet);

void parser_test_setup_file(struct parser_test_context *t_ctx,
			    const char *file_name);

mc_status_t parser_test_init(struct parser_test_context *t_ctx);

mc_status_t parser_test_parse_translation_unit(struct parser_test_context *t_ctx,
                                               struct pt_node **root);

void parser_test_free_tree(struct parser_test_context *t_ctx,
                           struct pt_node *root);

struct pt_node *parser_test_find_node_by_symbol(struct pt_node *root,
                                                enum parser_symbol sym,
                                                uint16_t occurrence);

struct token *parser_test_find_identifier(struct pt_node *root,
                                          const char *identifier,
                                          uint16_t occurrence);

void parser_test_free(struct parser_test_context *t_ctx);

#endif /* _COMMON_H_ */