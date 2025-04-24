#ifndef _IRGEN_TEST_UTIL_H_
#define _IRGEN_TEST_UTIL_H_

/* Global Project Headers */
#include <mc.h>
#include <fs.h>       /* For struct filesys */
#include <pp.h>       /* For struct pp_context */
#include <token.h>    /* For struct convert_context */
#include <parser.h>   /* For struct parser, parser_process_node_t */
#include <ir/gen.h>   /* For struct irgen_context */
#include <ir/module.h>/* For struct ir_module */
#include <ir/object.h>/* For struct ir_object */

/* Structure to hold the full environment for parser-based IR tests */
typedef struct {
    struct filesys fs;
    struct pp_context pp;
    struct convert_context convert_ctx;
    struct parser parser;
    struct parser_context { /* Mimic structure from main.c/parser_test.c */
        struct token *curr;
        struct token *last_pull;
        struct token *last_error;
    } parser_ctx;
    struct irgen_context gen_ctx;
    struct ir_module *module;     /* Convenience pointer to gen_ctx.mod */
    struct ir_object *result_obj; /* Convenience pointer to default result obj */
} ir_test_full_env_t;

/* Initializes the full pipeline (fs, pp, convert, parser, irgen) */
/* Writes 'code_snippet' to a temporary file for processing. */
mc_status_t ir_test_init_with_code(ir_test_full_env_t *full_env,
                   const char *code_snippet,
                   const char *module_name);

/* Parses the token stream in the environment using the specified parser function */
/* The resulting AST node is placed in out_node. */
/* The caller is responsible for calling ast_destroy on the out_node. */
mc_status_t ir_test_parse_node(ir_test_full_env_t *full_env,
                   parser_process_node_t parse_func,
                   struct pt_node **out_node);

/* Frees all resources allocated by ir_test_init_with_code */
void ir_test_free(ir_test_full_env_t *full_env);

/* Custom error handler for IR generation tests (can remain the same) */
mc_status_t test_irgen_error_handler(struct pt_node *node, const char *message);

void ast_destroy(struct pt_node *root);

#endif /* _IRGEN_TEST_UTIL_H_ */