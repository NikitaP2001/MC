#include "test_util.h"
#include <assert.h>
#include <ir.h>
#include <ir/scalar.h>
#include <mc.h>
#include <parser/ast.h>
#include <stdio.h>
#include <test_suite.h>

TEST_CASE(ir_gen, additive_expression_simple_const)
{
	struct ir_test_full_env full_env;
	struct pt_node *expr_node = NULL;
	const char *code = "1 + 2 + 432434 + 43";
	mc_status_t status;

	/* Initialize environment and parse code */
	status = ir_test_init_with_code(&full_env, code, NULL);
	if (status != MC_OK) {
		return;
	}

        struct parser *ps = ir_test_get_parser(&full_env);

	/* Parse expression */
	status = ir_test_parse_node(
	    &full_env, 
            ps->ops->expression, 
            &expr_node);
	if (!expr_node || status != MC_OK) {
		goto cleanup;
	}

	/* Generate IR for the expression */
	status = full_env.gen_ctx.ops->expression(&full_env.gen_ctx,
							   expr_node);
	EXPECT_EQ(status, MC_OK);

	/* Check result */
	ASSERT_TRUE(ir_obj_const_eval(full_env.result_obj));
        struct ir_scalar result = ir_obj_scalar_get(full_env.result_obj);
        EXPECT_EQ(result.type, s_i32);
        EXPECT_EQ(result.data.var_int, 432480);

cleanup:
	if (expr_node) {
                pt_node_destroy(expr_node);
	}
	ir_test_free(&full_env);
}

int main(int argc, char *argv[])
{
	mc_init(argc, argv);
	TEST_RUN(ir_gen, additive_expression_simple_const);
	mc_free();
	return TEST_RESULT;
}