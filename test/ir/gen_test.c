/* Standard Library Headers */
#include <stdio.h>
#include <assert.h>

/* Global Project Headers */
#include <mc.h>
#include <list.h>
#include <ir.h>
#include <ir/scalar.h>
#include <ir/object.h>
#include <parser/ast.h>
#include <parser/symbol.h>

/* Local Headers */
#include <test_suite.h>
#include "test_util.h"

TEST_CASE(ir_gen, additive_expression_simple_const)
{
    ir_test_full_env_t full_env;
    mc_status_t status;
    struct pt_node *add_expr = NULL;
    const char *code = "1 + 2"; /* Code snippet to parse */

    /* 1. Initialize Full Environment & Parse Code */
    status = ir_test_init_with_code(&full_env, code, "test_add_const");
    EXPECT_EQ(status, MC_OK);
    if (status != MC_OK) {
        /* No AST to destroy yet */
        return; /* Teardown happens implicitly via TEST_CASE macro */
    }

    /* Check pointers initialized by the helper */
    EXPECT_NE(full_env.module, NULL);
    EXPECT_NE(full_env.gen_ctx.ops, NULL);
    EXPECT_NE(full_env.result_obj, NULL);
    EXPECT_NE(full_env.parser.ops, NULL);

    /* Use the parser to generate the AST for the specific expression type */
    status = ir_test_parse_node(&full_env,
                    full_env.parser.ops->additive_expression,
                    &add_expr);
    EXPECT_EQ(status, MC_OK);
    EXPECT_NE(add_expr, NULL);
    if (!add_expr || status != MC_OK) {
        goto cleanup; /* Use goto for cleanup */
    }

    /* Optional: Add basic AST structure checks if desired */
    /* EXPECT_EQ(add_expr->sym, psym_additive_expression); */

    /* 2. Run the specific IR generation function using the parsed AST */
    status = full_env.gen_ctx.ops->additive_expression(&full_env.gen_ctx, add_expr);

    /* 3. Assertions on IR Output */
    EXPECT_EQ(status, MC_OK);
    EXPECT_TRUE(ir_obj_const_eval(full_env.result_obj));

    if (ir_obj_const_eval(full_env.result_obj)) {
        struct ir_scalar result_scalar =
            ir_obj_scalar_get(full_env.result_obj);
        EXPECT_EQ(result_scalar.type, s_i32);
        EXPECT_EQ(result_scalar.data.var_int, 3);
    } else {
        TEST_FAILTURE("Result object was not constant evaluable.\n");
    }

    /* Check if any unexpected functions were generated (should be none for const expr) */
    EXPECT_EQ(full_env.module->num_functions, 0);

cleanup:
    /* 4. Teardown */
    if (add_expr) {
        ast_destroy(add_expr); /* Destroy the parsed AST */
    }
    ir_test_free(&full_env); /* Free the full environment */
}

int main(int argc, char *argv[])
{
    /* Initialize the compiler module */
    mc_init(argc, argv);

    /* Run the test cases */
    TEST_RUN(ir_gen, additive_expression_simple_const);
    /* Add more TEST_RUN calls here */

    /* Free the compiler module resources */
    mc_free();

    return TEST_RESULT;
}