#include <stdio.h> /* Keep for remove() */
#include <stdlib.h>
#include <string.h>
#include <ir.h>
#include <mc.h>
#include <token.h>
#include <test_tools.h>
#include <common.h>

#include "test_util.h"

mc_status_t test_irgen_error_handler(struct pt_node *node, const char *message)
{
	struct pp_token *pp_tok = NULL;
	const char *file_path = "<unknown>";
	unsigned long file_line = 0;

	/* Use MC_DBG with MC_ERR level for IR generation errors */
	if (node && node->node_value.value && node->node_value.value->first) {
		pp_tok = node->node_value.value->first;
		if (pp_tok->src_file) {
			file_path = pp_tok->src_file->path;
			file_line = (unsigned long)pp_tok->file_line;
			MC_DBG(MC_ERR, "IRGEN TEST ERROR: %s (near %s:%lu)",
			       message, file_path, file_line);
		} else {
			MC_DBG(MC_ERR,
			       "IRGEN TEST ERROR: %s (near token with no "
			       "source file info)",
			       message);
		}
	} else {
		MC_DBG(MC_ERR,
		       "IRGEN TEST ERROR: %s (no node or token info available)",
		       message);
	}

	return MC_FAIL;
}

mc_status_t ir_test_init_with_code(struct ir_test_full_env *full_env,
				   const char *code_snippet,
				   const char *module_name)
{
	memset(full_env, 0, sizeof(struct ir_test_full_env));
        parser_test_setup_snippet(&full_env->p_ctx, code_snippet);
        parser_test_init(&full_env->p_ctx);

        struct parser *ps = ir_test_get_parser(full_env);
	irgen_init(&full_env->gen_ctx, ps);
	full_env->gen_ctx.error = test_irgen_error_handler;

	full_env->module = full_env->gen_ctx.mod;
	if (!full_env->module) {
		MC_DBG(MC_ERR, "IR module not created by irgen_init");
		return MC_FAIL;
	}
	UNUSED(module_name);

	if (!full_env->gen_ctx.ops) {
		MC_DBG(MC_ERR, "IRGen Ops not initialized!");
		return MC_FAIL;
	}

	full_env->result_obj = irgen_obj_create(&full_env->gen_ctx, NULL);
	if (!full_env->result_obj) {
		MC_DBG(MC_ERR, "Failed to create result object");
		return MC_FAIL;
	}
	irgen_value_obj_set(&full_env->gen_ctx, full_env->result_obj);

	return MC_OK;
}

mc_status_t ir_test_parse_node(struct ir_test_full_env *full_env,
			       parser_process_node_t parse_func,
			       struct pt_node **out_node)
{
	mc_status_t status;

	if (!parse_func) {
		return MC_FAIL;
	}

	/* Ensure lookahead is NULL before parsing a specific fragment */
        struct parser *ps = ir_test_get_parser(full_env);
	if (ps->lookahead) {
		pt_node_destroy(ps->lookahead);
		ps->lookahead = NULL;
	}

	status = parse_func(ps);
	if (!MC_SUCC(status)) {
		*out_node = NULL;
		return status; /* Parser error occurred */
	}

	*out_node = parser_result_pull(ps);
	if (!*out_node) {
		/* This shouldn't happen if status was OK, but check anyway */
		MC_DBG(
		    MC_ERR,
		    "parser_result_pull returned NULL after successful parse");
		return MC_FAIL;
	}

	return MC_OK;
}

void ir_test_free(struct ir_test_full_env *full_env)
{
	irgen_free(&full_env->gen_ctx);
        parser_test_free(&full_env->p_ctx);
}
	