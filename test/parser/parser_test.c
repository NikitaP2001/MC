#include <stdio.h>
#include <stdlib.h>
     
#include <mc.h>
#include <parser.h>
#include <test_suite.h>
#include <common.h>

static _Bool parser_test_case_file(const char *name)
{
        _Bool result = true;
        struct parser_test_context t_ctx;
        parser_test_setup_file(&t_ctx, name);
        if (!MC_SUCC(parser_test_init(&t_ctx)))
                return false;
        struct parser *ps = &t_ctx.parser;
        mc_status_t status = ps->ops->translation_unit(ps);
        if (!MC_SUCC(status))
                result = false;
        struct pt_node *node = parser_result_pull(ps);
        if (node == NULL)
                result = false;
        else
                pt_node_destroy(node);
        parser_test_free(&t_ctx);
        return result;
}

TEST_CASE(parser, general_main_1)
{
        ASSERT_TRUE(parser_test_case_file("test1.tc"));
}

TEST_CASE(parser, typedef_valid)
{
        ASSERT_TRUE(parser_test_case_file("test2.tc"));
}

TEST_CASE(parser, typedef_invalid)
{
        ASSERT_FALSE(parser_test_case_file("test3.tc"));
}

TEST_CASE(parser, local_var_valid)
{
        ASSERT_TRUE(parser_test_case_file("test4.tc"));
}

TEST_CASE(parser, func_def_valid)
{
        ASSERT_TRUE(parser_test_case_file("test5.tc"));
}

TEST_CASE(parser, declarator_valid)
{
        ASSERT_TRUE(parser_test_case_file("test6.tc"));
}

TEST_CASE(parser, abstract_declarator_valid)
{
        ASSERT_TRUE(parser_test_case_file("test7.tc"));
}

TEST_CASE(parser, declarator_invalid)
{
        ASSERT_FALSE(parser_test_case_file("test8.tc"));
}

TEST_CASE(parser, abstract_declarator_invalid)
{
        ASSERT_FALSE(parser_test_case_file("test9.tc"));
}

TEST_CASE(parser, struct_declaration_valid)
{
        ASSERT_TRUE(parser_test_case_file("test10.tc"));
}

TEST_CASE(parser, struct_empty_invalid)
{
        ASSERT_FALSE(parser_test_case_file("test11.tc"));
}

static inline struct pt_node *parser_test_get_subexpr(struct pt_node *expr_node,
                                            enum parser_symbol sym)
{
        struct pt_node *subexpr = NULL;
        while ((expr_node = pt_node_child_first(expr_node))) {
                if (expr_node->sym == sym)
                        subexpr = expr_node;
        }
        return subexpr;
}

TEST_CASE(parser, additive_expression_simple)
{
        const char *code_snippet = "1+2";
        struct parser_test_context t_ctx;
        struct pt_node *expr_node = NULL;
        mc_status_t status;

        parser_test_setup_snippet(&t_ctx, code_snippet);
        status = parser_test_init(&t_ctx);
        ASSERT_TRUE(MC_SUCC(status));

        struct parser *ps = &t_ctx.parser;
        status = ps->ops->constant_expression(ps);
        ASSERT_TRUE(MC_SUCC(status));

        expr_node = parser_test_get_subexpr(parser_result_pull(ps), 
                psym_additive_expression);
        ASSERT_NE(expr_node, NULL);

        EXPECT_EQ(expr_node->sym, psym_additive_expression);
        EXPECT_EQ(pt_node_child_count(expr_node), 3);

        struct pt_node *left_operand_expr = pt_node_child_first(expr_node);
        struct pt_node *op_node = pt_node_child_number(expr_node, 1);
        struct pt_node *right_operand_expr = pt_node_child_last(expr_node);

        ASSERT_NE(left_operand_expr, NULL);
        ASSERT_NE(op_node, NULL);
        ASSERT_NE(right_operand_expr, NULL);

        EXPECT_EQ(left_operand_expr->sym, psym_multiplicative_expression);
        EXPECT_EQ(right_operand_expr->sym, psym_multiplicative_expression);

        struct pt_node *left_const_node = parser_test_get_subexpr(
                left_operand_expr, psym_constant);
        ASSERT_NE(left_const_node, NULL);
        struct pt_node *right_const_node = parser_test_get_subexpr(
                right_operand_expr, psym_constant);
        ASSERT_NE(right_const_node, NULL);

        struct token *left_token = left_const_node ->node_value.value;
        ASSERT_NE(left_token, NULL);
        EXPECT_EQ(left_token->value.var_const.type, const_int);
        EXPECT_EQ(left_token->value.var_const.data.var_int, 1);

        struct token *right_token = right_const_node->node_value.value;
        ASSERT_NE(right_token , NULL);
        EXPECT_EQ(right_token->value.var_const.type, const_int);
        EXPECT_EQ(right_token->value.var_const.data.var_int, 2);
        
        pt_node_destroy(expr_node);
        parser_test_free(&t_ctx);
}

int main(int argc, char *argv[])
{
     mc_init(argc, argv);
     TEST_RUN(parser, general_main_1);
     TEST_RUN(parser, typedef_valid);
     TEST_RUN(parser, typedef_invalid);
     TEST_RUN(parser, local_var_valid);
     TEST_RUN(parser, func_def_valid);
     TEST_RUN(parser, declarator_valid);
     TEST_RUN(parser, abstract_declarator_valid);
     TEST_RUN(parser, declarator_invalid);
     TEST_RUN(parser, abstract_declarator_invalid);
     TEST_RUN(parser, struct_declaration_valid);
     TEST_RUN(parser, struct_empty_invalid);
     TEST_RUN(parser, additive_expression_simple);

     mc_free();
     return TEST_RESULT;
}