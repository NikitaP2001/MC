#include <stdio.h>

#include <pp.h>
#include <fs.h>
#include <token.h>
#include <test_common.h>

TEST_CASE(token, wstrl_uchr_names)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     const char test[] = "L\" \\U00001234 \\U00012345 \""
                         "\" \\u5f12 \\u0024 \\u0512 \"";
     uint8_t res[] = { 0x20, 0x0, 0x34, 0x12, 0x20, 0x0, 0x8, 0xd8, 0x45, 
                       0xdf, 0x20, 0x0, 0x20, 0x0, 0x12, 0x5f, 0x20, 0x0, 
                       0x24, 0x0, 0x20, 0x0, 0x12, 0x5, 0x20, 0x0, 0x0, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, strl_uchr_names)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
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

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, winteger_esc_seq)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
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

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, integer_esc_seq)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
     fs_init(&fs);
     fs_add_local(&fs, "./");

     pp_init(&pp, &fs);

     char test[] = "\" \\123 \\7 \\65 \\x65 \\xf1 \"";
     uint8_t res[] = { 0x20, 0x53, 0x20, 0x7, 0x20, 0x35, 0x20, 
                       0x65, 0x20, 0xf1, 0x20, 0x0 };
     ASSERT_TRUE(write_file(TFILE_NAME, test, LEN_OF(test)));
     enum mc_status status = pp_run(&pp, TFILE_NAME);
     ASSERT_TRUE(MC_SUCC(status));

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, wsimple_esc_seq)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
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

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, simple_esc_seq)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
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

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, concat_strlit)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
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

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);
     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}

TEST_CASE(token, concat_wstrlit)
{
     mc_init();
     struct filesys fs;
     struct pp_context pp;
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

     struct token *tok = token_convert(&pp);
     ASSERT_NE(tok, NULL);
     EXPECT_EQ(tok->type, tok_strlit);
     EXPECT_EQ(tok->value.var_raw.length, sizeof(res));
     EXPECT_STR_EQ(tok->value.var_raw.value, (char*)res);

     pp_free(&pp);
     fs_free(&fs);
     remove(TFILE_NAME);
}


int main()
{
     TEST_RUN(token, wstrl_uchr_names);
     TEST_RUN(token, strl_uchr_names);
     TEST_RUN(token, integer_esc_seq);
     TEST_RUN(token, winteger_esc_seq);
     TEST_RUN(token, simple_esc_seq);
     TEST_RUN(token, wsimple_esc_seq);
     TEST_RUN(token, concat_strlit);
     TEST_RUN(token, concat_wstrlit);
}