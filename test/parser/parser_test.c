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

TEST_CASE(parser, typedef_use_before_declaration_invalid)
{
        ASSERT_FALSE(parser_test_case_file("test14.tc"));
}

TEST_CASE(parser, parameter_identifier_not_typedef_invalid)
{
        ASSERT_FALSE(parser_test_case_file("test16.tc"));
}

TEST_CASE(parser, abstract_declarator_additional_valid)
{
        ASSERT_TRUE(parser_test_case_file("test17.tc"));
}

TEST_CASE(parser, abstract_declarator_additional_invalid)
{
        ASSERT_FALSE(parser_test_case_file("test18.tc"));
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

TEST_CASE(parser, declaration_visibility_direct)
{
        const char *code_snippet =
                "int main() {\n"
                "    int outer = 0;\n"
                "    {\n"
                "        int inner = outer;\n"
                "    }\n"
                "    outer = 1;\n"
                "    return outer;\n"
                "}\n";
        struct parser_test_context t_ctx;
        struct pt_node *root = NULL;

        parser_test_setup_snippet(&t_ctx, code_snippet);
        ASSERT_TRUE(MC_SUCC(parser_test_init(&t_ctx)));
        ASSERT_TRUE(MC_SUCC(parser_test_parse_translation_unit(&t_ctx, &root)));

        struct pt_node *fn_scope = parser_test_find_node_by_symbol(
                root, psym_compound_statement, 1);
        struct pt_node *inner_scope = parser_test_find_node_by_symbol(
                root, psym_compound_statement, 2);
        ASSERT_NE(fn_scope, NULL);
        ASSERT_NE(inner_scope, NULL);

        struct token *tok_outer = parser_test_find_identifier(root, "outer", 1);
        struct token *tok_inner = parser_test_find_identifier(root, "inner", 1);
        ASSERT_NE(tok_outer, NULL);
        ASSERT_NE(tok_inner, NULL);

        struct declaration *decl_outer_in_inner = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_outer, inner_scope);
        struct declaration *decl_inner_in_inner = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_inner, inner_scope);
        struct declaration *decl_inner_in_fn = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_inner, fn_scope);

        EXPECT_NE(decl_outer_in_inner, NULL);
        EXPECT_NE(decl_inner_in_inner, NULL);
        EXPECT_EQ(decl_inner_in_fn, NULL);
        EXPECT_TRUE((decl_outer_in_inner->specs.type_spec & ts_int) != 0);
        EXPECT_TRUE((decl_inner_in_inner->specs.type_spec & ts_int) != 0);
        EXPECT_EQ(decl_outer_in_inner->specs.storage_class, scs_none);
        EXPECT_EQ(decl_inner_in_inner->specs.storage_class, scs_none);

        parser_test_free_tree(&t_ctx, root);
}

TEST_CASE(parser, typedef_visibility_direct)
{
        const char *code_snippet =
                "typedef int T;\n"
                "int main() {\n"
                "    T a;\n"
                "    {\n"
                "        typedef char T;\n"
                "        T b;\n"
                "    }\n"
                "    T c;\n"
                "    return 0;\n"
                "}\n";
        struct parser_test_context t_ctx;
        struct pt_node *root = NULL;

        parser_test_setup_snippet(&t_ctx, code_snippet);
        ASSERT_TRUE(MC_SUCC(parser_test_init(&t_ctx)));
        ASSERT_TRUE(MC_SUCC(parser_test_parse_translation_unit(&t_ctx, &root)));

        struct pt_node *fn_scope = parser_test_find_node_by_symbol(
                root, psym_compound_statement, 1);
        struct pt_node *inner_scope = parser_test_find_node_by_symbol(
                root, psym_compound_statement, 2);
        ASSERT_NE(fn_scope, NULL);
        ASSERT_NE(inner_scope, NULL);

        struct token *tok_t = parser_test_find_identifier(root, "T", 1);
        ASSERT_NE(tok_t, NULL);

        struct declaration *outer_t = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_t, fn_scope);
        struct declaration *inner_t = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_t, inner_scope);

        ASSERT_NE(outer_t, NULL);
        ASSERT_NE(inner_t, NULL);
        EXPECT_TRUE((outer_t->specs.storage_class & scs_typedef) != 0);
        EXPECT_TRUE((inner_t->specs.storage_class & scs_typedef) != 0);
        EXPECT_TRUE((outer_t->specs.type_spec & ts_int) != 0);
        EXPECT_TRUE((inner_t->specs.type_spec & ts_char) != 0);
        EXPECT_NE(outer_t, inner_t);

        parser_test_free_tree(&t_ctx, root);
}

TEST_CASE(parser, parameter_visibility_direct)
{
        const char *code_snippet =
                "int foo(int p) {\n"
                "    {\n"
                "        int q = p;\n"
                "    }\n"
                "    return p;\n"
                "}\n";
        struct parser_test_context t_ctx;
        struct pt_node *root = NULL;

        parser_test_setup_snippet(&t_ctx, code_snippet);
        ASSERT_TRUE(MC_SUCC(parser_test_init(&t_ctx)));
        ASSERT_TRUE(MC_SUCC(parser_test_parse_translation_unit(&t_ctx, &root)));

        struct pt_node *fn_scope = parser_test_find_node_by_symbol(
                root, psym_compound_statement, 1);
        struct pt_node *inner_scope = parser_test_find_node_by_symbol(
                root, psym_compound_statement, 2);
        ASSERT_NE(fn_scope, NULL);
        ASSERT_NE(inner_scope, NULL);

        struct token *tok_p = parser_test_find_identifier(root, "p", 1);
        struct token *tok_q = parser_test_find_identifier(root, "q", 1);
        ASSERT_NE(tok_p, NULL);
        ASSERT_NE(tok_q, NULL);

        struct declaration *p_fn = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_p, fn_scope);
        struct declaration *p_inner = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_p, inner_scope);
        struct declaration *q_inner = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_q, inner_scope);
        struct declaration *q_fn = symtable_get_declaration(
                &t_ctx.parser.sym_tbl, tok_q, fn_scope);

        EXPECT_NE(p_fn, NULL);
        EXPECT_NE(p_inner, NULL);
        EXPECT_NE(q_inner, NULL);
        EXPECT_EQ(q_fn, NULL);
        EXPECT_TRUE((p_fn->specs.type_spec & ts_int) != 0);
        EXPECT_TRUE((q_inner->specs.type_spec & ts_int) != 0);
        EXPECT_EQ(p_fn->specs.storage_class, scs_none);
        EXPECT_EQ(q_inner->specs.storage_class, scs_none);

        parser_test_free_tree(&t_ctx, root);
}

TEST_CASE(parser, label_visibility_direct)
{
        const char *code_snippet =
                "int f() {\n"
                "    goto L;\n"
                "L:\n"
                "    return 0;\n"
                "}\n"
                "int g() {\n"
                "    return 0;\n"
                "}\n";
        struct parser_test_context t_ctx;
        struct pt_node *root = NULL;

        parser_test_setup_snippet(&t_ctx, code_snippet);
        ASSERT_TRUE(MC_SUCC(parser_test_init(&t_ctx)));
        ASSERT_TRUE(MC_SUCC(parser_test_parse_translation_unit(&t_ctx, &root)));

        struct pt_node *f_scope = parser_test_find_node_by_symbol(
                root, psym_function_definition, 1);
        struct pt_node *g_scope = parser_test_find_node_by_symbol(
                root, psym_function_definition, 2);
        ASSERT_NE(f_scope, NULL);
        ASSERT_NE(g_scope, NULL);

        struct token *tok_l = parser_test_find_identifier(root, "L", 1);
        ASSERT_NE(tok_l, NULL);

        struct label *lbl_in_f = symtable_get_label(&t_ctx.parser.sym_tbl,
                tok_l, f_scope);
        struct label *lbl_in_g = symtable_get_label(&t_ctx.parser.sym_tbl,
                tok_l, g_scope);

        EXPECT_NE(lbl_in_f, NULL);
        EXPECT_EQ(lbl_in_g, NULL);

        parser_test_free_tree(&t_ctx, root);
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
        TEST_RUN(parser, typedef_use_before_declaration_invalid);
        TEST_RUN(parser, parameter_identifier_not_typedef_invalid);
        TEST_RUN(parser, abstract_declarator_additional_valid);
        TEST_RUN(parser, abstract_declarator_additional_invalid);
        TEST_RUN(parser, additive_expression_simple);
        TEST_RUN(parser, declaration_visibility_direct);
        TEST_RUN(parser, typedef_visibility_direct);
        TEST_RUN(parser, parameter_visibility_direct);
        TEST_RUN(parser, label_visibility_direct);

        mc_free();
        return TEST_RESULT;
}