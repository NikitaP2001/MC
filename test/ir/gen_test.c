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

/* test the result of expression as seq of assignment expressions */

static inline struct pt_node *irgen_node_identifier(struct pt_node *node)
{
        while (node != NULL) {
                if (pt_node_sym_cmp(node, psym_identifier))
                        return node;
                node = pt_node_child_first(node);
        }
        return NULL;
}

/* test const assignment expression */
TEST_CASE(ir_gen, assignment_expression_simple_const)
{
	struct ir_test_full_env full_env;
	struct pt_node *expr_node = NULL;
	const char *code = "a = 42";
	mc_status_t status;

	/* Initialize environment and parse code */
	status = ir_test_init_with_code(&full_env, code, NULL);
	if (status != MC_OK) {
		return;
	}

	struct parser *ps = ir_test_get_parser(&full_env);

	/* Parse assignment expression */
	status = ir_test_parse_node(
	    &full_env, 
	    ps->ops->assignment_expression, 
	    &expr_node);
	if (!expr_node || status != MC_OK) {
		goto cleanup;
	}

	/* Manually create variable 'a' using the parsed identifier node */
        struct pt_node *id_node = irgen_node_identifier(expr_node);
	struct ir_object *obj_a = irgen_obj_create(&full_env.gen_ctx, id_node);
	struct ir_scalar a_type = {
		.type = s_i32,
		.is_signed = true,
	};
	ir_obj_scalar_set(obj_a, a_type);

	/* Generate IR for the assignment expression */
	status = full_env.gen_ctx.ops->assignment_expression(&full_env.gen_ctx,
							     expr_node);
	ASSERT_EQ(status, MC_OK);

	/* Check result - should be the assigned value */
	ASSERT_TRUE(ir_obj_const_eval(full_env.result_obj));
	struct ir_scalar result = ir_obj_scalar_get(full_env.result_obj);
	EXPECT_EQ(result.type, s_i32);
	EXPECT_EQ(result.data.var_int, 42);

        struct ir_value *var_a = irgen_context_value_get(
                &full_env.gen_ctx,
                pt_node_child_first(expr_node));
        struct ir_object *var_a_obj = ir_value_object_get(var_a);
        EXPECT_NE(var_a_obj, NULL);
        ASSERT_TRUE(ir_obj_const_eval(var_a_obj));
        result = ir_obj_scalar_get(var_a_obj);
        EXPECT_EQ(result.type, s_i32);
        EXPECT_EQ(result.data.var_int, 42);

cleanup:
	if (expr_node) {
		pt_node_destroy(expr_node);
	}
	ir_test_free(&full_env);
}

/* test dynamic assignment expression */
TEST_CASE(ir_gen, assignment_expression_simple_dyn)
{
	struct ir_test_full_env full_env;
	struct pt_node *proc_node = NULL;
	const char *code = "void proc(int a) { a = 42; }";
	mc_status_t status;

	/* Initialize environment and parse code */
	status = ir_test_init_with_code(&full_env, code, NULL);
	if (status != MC_OK) {
		return;
	}

	struct parser *ps = ir_test_get_parser(&full_env);

	/* Parse assignment expression */
	status = ir_test_parse_node(
	    &full_env, 
	    ps->ops->external_declaration, 
	    &proc_node);
	if (!proc_node || status != MC_OK) {
		goto cleanup;
	}
        status = full_env.gen_ctx.ops->external_declaration(&full_env.gen_ctx,
							     proc_node);
	ASSERT_EQ(status, MC_OK);

cleanup:
	if (proc_node) {
		pt_node_destroy(proc_node);
	}

	ir_test_free(&full_env);
}

/* test conditional expression */

int main(int argc, char *argv[])
{
	mc_init(argc, argv);
	TEST_RUN(ir_gen, additive_expression_simple_const);
	TEST_RUN(ir_gen, assignment_expression_simple_const);
        TEST_RUN(ir_gen, assignment_expression_simple_dyn);
	mc_free();
	return TEST_RESULT;
}