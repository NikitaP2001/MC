#ifndef _IRGEN_TEST_UTIL_H_
#define _IRGEN_TEST_UTIL_H_

#include <mc.h>
#include <fs.h>
#include <pp.h>
#include <token.h>
#include <parser.h>
#include <ir/gen.h>
#include <ir/module.h>
#include <ir/object.h>
#include <common.h>

struct ir_test_full_env {
        struct parser_test_context p_ctx;
        struct irgen_context gen_ctx;
        struct ir_module *module;
        struct ir_object *result_obj;
};

mc_status_t ir_test_init_with_code(struct ir_test_full_env *full_env,
                   const char *code_snippet,
                   const char *module_name);

mc_status_t ir_test_parse_node(struct ir_test_full_env *full_env,
                   parser_process_node_t parse_func,
                   struct pt_node **out_node);

void ir_test_free(struct ir_test_full_env *full_env);

mc_status_t test_irgen_error_handler(struct pt_node *node, const char *message);

static inline struct parser *ir_test_get_parser(struct ir_test_full_env *full_env)
{
        return &full_env->p_ctx.parser;
}

#endif /* _IRGEN_TEST_UTIL_H_ */