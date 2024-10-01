#include <stdio.h>
#include <stdlib.h>

#include <pp.h>
#include <fs.h>
#include <token.h>
#include <test_common.h>

#define RAW_VAL_CMP(token, t_val)                             \
{                                                             \
     EXPECT_EQ(token->type, tok_identifier);                  \
     EXPECT_EQ(token->value.var_raw.length, LEN_OF(t_val));   \
     EXPECT_EQ(memcmp(token->value.var_raw.value, t_val,      \
          LEN_OF(t_val)), 0);                                 \
}

TEST_CASE(token, numbers_float_valid)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");
     pp_init(&pp, &fs);

     const char test[] = " 123.12 123.f ";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_double)
     ASSERT_EQ(tok->value.var_const.data.var_long_double, 123.12L);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_float)
     ASSERT_EQ(tok->value.var_const.data.var_long_double, 123.L);
     
     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, int_suffix_invalid)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");
     pp_init(&pp, &fs);

     const char test[] = " 1234lll ";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_FALSE(MC_SUCC(convert_run(&ctx)));
     
     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, numbers_int_valid)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");
     pp_init(&pp, &fs);

     const char *format ="%d %ldl %lldLL "
                         "0x%x 0x%lxl 0x%llxLL "
                         "%uu %lulu %llullu";

     int size = snprintf(NULL, 0, format, 
          INT_MAX - 1, LONG_MAX - 1, LLONG_MAX - 1, 
          INT_MAX - 1, LONG_MAX - 1, LLONG_MAX - 1, 
          UINT_MAX - 1, ULONG_MAX - 1, ULONG_LONG_MAX - 1);
     char *test = malloc(size + 1);
     snprintf(test, size + 1, format,
          INT_MAX - 1, LONG_MAX - 1, LLONG_MAX - 1, 
          INT_MAX - 1, LONG_MAX - 1, LLONG_MAX - 1, 
          UINT_MAX - 1, ULONG_MAX - 1, ULONG_LONG_MAX - 1);

     ASSERT_TRUE(write_file(TFILE_NAME, test, strlen(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_int)
     ASSERT_EQ(tok->value.var_const.data.var_int, INT_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_long_int);
     ASSERT_EQ(tok->value.var_const.data.var_int, LONG_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_long_long_int);
     ASSERT_EQ(tok->value.var_const.data.var_int, LLONG_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_int);
     ASSERT_EQ(tok->value.var_const.data.var_int, INT_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_long_int);
     ASSERT_EQ(tok->value.var_const.data.var_int, LONG_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_long_long_int);
     ASSERT_EQ(tok->value.var_const.data.var_int, LLONG_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_uint);
     ASSERT_EQ(tok->value.var_const.data.var_uint, UINT_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_ulong_int);
     ASSERT_EQ(tok->value.var_const.data.var_uint, ULONG_MAX - 1);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_constant);
     ASSERT_EQ(tok->value.var_const.type, const_ulong_long_int);
     ASSERT_EQ(tok->value.var_const.data.var_uint, ULONG_LONG_MAX - 1);   
     
     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, punct_after_strlit)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");
     pp_init(&pp, &fs);

     const char test[] = "\"Hello, World!\";";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);

     EXPECT_EQ(tok->type, tok_strlit)
     tok = list_next(tok); /* str literal */
     EXPECT_EQ(tok->type, tok_punctuator);
     EXPECT_EQ(tok->value.var_punc, punc_semicolon)

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, punct_seq)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");
     pp_init(&pp, &fs);

     const char test[] = "; [] <<= ; +=";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_EQ(tok->type, tok_punctuator);
     ASSERT_EQ(tok->value.var_punc, punc_semicolon)

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_punctuator);
     ASSERT_EQ(tok->value.var_punc, punc_left_sq_br);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_punctuator);
     ASSERT_EQ(tok->value.var_punc, punc_right_sq_br);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_punctuator);
     ASSERT_EQ(tok->value.var_punc, punc_shl_assign);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_punctuator);
     ASSERT_EQ(tok->value.var_punc, punc_semicolon);

     tok = list_next(tok);
     ASSERT_EQ(tok->type, tok_punctuator);
     ASSERT_EQ(tok->value.var_punc, punc_add_assign);
     
     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, invalid_token_identifier)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "\\u0032ident";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_FALSE(MC_SUCC(convert_run(&ctx)));
     
     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, valid_token_sequence)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "ident1 char ident\\u0032";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     RAW_VAL_CMP(tok, "ident1");

     tok = (struct token *)list_next(tok);
     EXPECT_EQ(tok->type, tok_keyword);
     EXPECT_EQ(tok->value.var_keyw, keyw_char);

     tok = (struct token *)list_next(tok);
     RAW_VAL_CMP(tok, "ident\x32");

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, strlit_no_end)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "\"no end quote";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_FALSE(MC_SUCC(convert_run(&ctx)));

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, strlit_multiline)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "\"first line \"\n"
                         "\"second\"";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_STR_EQ(tok->value.var_raw.value, "first line second");

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, wstrl_uchr_names)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "L\" \\U00001234 \\U00012345 \""
                         "\" \\u5f12 \\u0024 \\u0512 \"";
     uint8_t res[] = { 0x20, 0x0, 0x34, 0x12, 0x20, 0x0, 0x8, 0xd8, 0x45, 
                       0xdf, 0x20, 0x0, 0x20, 0x0, 0x12, 0x5f, 0x20, 0x0, 
                       0x24, 0x0, 0x20, 0x0, 0x12, 0x5, 0x20, 0x0, 0x0, 
                       0x0 };

     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, strl_uchr_names)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "\" \\U00001234 \\U00012345 \""
                         "\" \\u5f12 \\u0024 \\u0512 \"";
     uint8_t res[] = { 0x20, 0xe1, 0x88, 0xb4, 0x20, 0xf0, 0x92, 
                       0x8d, 0x85, 0x20, 0x20, 0xe5, 0xbc, 0x92, 
                       0x20, 0x24, 0x20, 0xd4, 0x92, 0x20, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, winteger_esc_seq)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     char test[] = "L\" \\123 \\7 \\65 \\x65 \\xf1 \"";
     uint8_t res[] = { 0x20, 0x0, 0x53, 0x0, 0x20, 0x0, 0x7, 0x0, 0x20, 0x0, 
                       0x35, 0x0, 0x20, 0x0, 0x65, 0x0, 0x20, 0x0, 0xf1, 0x0, 
                       0x20, 0x0, 0x0, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, integer_esc_seq)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     char test[] = "\" \\123 \\7 \\65 \\x65 \\xf1 \"";
     uint8_t res[] = { 0x20, 0x53, 0x20, 0x7, 0x20, 0x35, 0x20, 
                       0x65, 0x20, 0xf1, 0x20, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, wsimple_esc_seq)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     char test[] =  "L\" \\a \\b \\\\ \\f \\n \\r \\t \\v \\' \\\" \\? \"";
     uint8_t res[] = { 0x20, 0x0, 0x7, 0x0, 0x20, 0x0, 0x8, 0x0, 0x20, 0x0, 
                       0x5c, 0x0, 0x20, 0x0, 0xc, 0x0, 0x20, 0x0, 0xa, 0x0, 
                       0x20, 0x0, 0xd, 0x0, 0x20, 0x0, 0x9, 0x0, 0x20, 0x0, 
                       0xb, 0x0, 0x20, 0x0, 0x27, 0x0, 0x20, 0x0, 0x22, 0x0, 
                       0x20, 0x0, 0x3f, 0x0, 0x20, 0x0, 0x0, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, simple_esc_seq)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     char test[] =  "\" \\a \\b \\\\ \\f \\n \\r \\t \\v \\' \\\" \\? \"";
     uint8_t res[] = { 0x20, 0x7, 0x20, 0x8, 0x20, 0x5c, 0x20, 
                       0xc, 0x20, 0xa, 0x20, 0xd, 0x20, 0x9, 
                       0x20, 0xb, 0x20, 0x27, 0x20, 0x22, 
                       0x20, 0x3f, 0x20, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, concat_strlit)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "\" first line \"\"second line\""
                         "\" third line \"";
     uint8_t res[] = { 0x20, 0x66, 0x69, 0x72, 0x73, 0x74, 0x20, 
                       0x6c, 0x69, 0x6e, 0x65, 0x20, 0x73, 0x65, 
                       0x63, 0x6f, 0x6e, 0x64, 0x20, 0x6c, 0x69, 
                       0x6e, 0x65, 0x20, 0x74, 0x68, 0x69, 0x72, 
                       0x64, 0x20, 0x6c, 0x69, 0x6e, 0x65, 0x20, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, concat_wstrlit)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     char test[] =  "L\" first line \"\"second line\""
                    "\" third line \"";
     uint8_t res[] = { 0x20, 0x0, 0x66, 0x0, 0x69, 0x0, 0x72, 0x0, 0x73, 
                       0x0, 0x74, 0x0, 0x20, 0x0, 0x6c, 0x0, 0x69, 0x0, 
                       0x6e, 0x0, 0x65, 0x0, 0x20, 0x0, 0x73, 0x0, 0x65, 
                       0x0, 0x63, 0x0, 0x6f, 0x0, 0x6e, 0x0, 0x64, 0x0, 
                       0x20, 0x0, 0x6c, 0x0, 0x69, 0x0, 0x6e, 0x0, 0x65, 
                       0x0, 0x20, 0x0, 0x74, 0x0, 0x68, 0x0, 0x69, 0x0, 
                       0x72, 0x0, 0x64, 0x0, 0x20, 0x0, 0x6c, 0x0, 0x69, 
                       0x0, 0x6e, 0x0, 0x65, 0x0, 0x20, 0x0, 0x0, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, char_literal)
{
     struct filesys fs;
     struct pp_context pp;
     struct convert_context ctx;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     char test[] =  "'1' '\\a' '1234' L'1' L'\\x12'";
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     convert_init(&ctx, &pp);
     ASSERT_TRUE(MC_SUCC(convert_run(&ctx)));
     struct token *tok = convert_get_token(&ctx);
     EXPECT_EQ(tok->type, tok_constant);
     EXPECT_EQ(tok->value.var_const.type, const_char);
     EXPECT_EQ(tok->value.var_const.data.var_int, '1');

     tok = list_next(tok);
     EXPECT_EQ(tok->type, tok_constant);
     EXPECT_EQ(tok->value.var_const.type, const_char);
     EXPECT_EQ(tok->value.var_const.data.var_int, '\a');

     tok = list_next(tok);
     EXPECT_EQ(tok->type, tok_constant);
     EXPECT_EQ(tok->value.var_const.type, const_char);
     EXPECT_EQ(tok->value.var_const.data.var_int, '1234');

     tok = list_next(tok);
     EXPECT_EQ(tok->type, tok_constant);
     EXPECT_EQ(tok->value.var_const.type, const_wchar_t);
     EXPECT_EQ(tok->value.var_const.data.var_int, L'1');

     tok = list_next(tok);
     EXPECT_EQ(tok->type, tok_constant);
     EXPECT_EQ(tok->value.var_const.type, const_wchar_t);
     EXPECT_EQ(tok->value.var_const.data.var_int, L'\x12');

     convert_free(&ctx);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

int main()
{
     mc_init();
     TEST_RUN(token, char_literal);
     TEST_RUN(token, numbers_float_valid);
     TEST_RUN(token, int_suffix_invalid);
     TEST_RUN(token, numbers_int_valid);
     TEST_RUN(token, punct_seq);
     TEST_RUN(token, punct_after_strlit);
     TEST_RUN(token, invalid_token_identifier);
     TEST_RUN(token, valid_token_sequence);
     TEST_RUN(token, wstrl_uchr_names);
     TEST_RUN(token, strl_uchr_names);
     TEST_RUN(token, strlit_no_end);
     TEST_RUN(token, strlit_multiline);
     TEST_RUN(token, integer_esc_seq);
     TEST_RUN(token, winteger_esc_seq);
     TEST_RUN(token, simple_esc_seq);
     TEST_RUN(token, wsimple_esc_seq);
     TEST_RUN(token, concat_strlit);
     TEST_RUN(token, concat_wstrlit);
     mc_free();
}