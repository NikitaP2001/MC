#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>

#include <gtest/gtest.h>

extern "C" {
        #include <pp.h>
        #include <fs.h>
        #include <tools.h>
        #include <testool.h>
}

#define FILE_NAME "src.c"

TEST(pp_test, header_name)
{
        const char file_name[] = FILE_NAME;
        const char test_text[] = "0.3< \n#include <name>";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys fs = {};
        fs_init(&fs);
        fs_add_local(&fs, "./");
        struct fs_file *src = fs_get_local(&fs, file_name);
        const char *srctext = source_read(src);
        ASSERT_NE(srctext, nullptr);

        struct preproc pp = {};
        pp_init(&pp, src);

        while (pp_get_token(&pp));
        struct pp_token *tok = pp.first;  /* 0.3 */
        ASSERT_NE(tok, nullptr);
        tok = (pp_token*)list_next(tok); /* < */
        ASSERT_NE(tok, nullptr);
        tok = (pp_token*)list_next(tok); /* # */
        ASSERT_NE(tok, nullptr);
        tok = (pp_token*)list_next(tok); /* include */
        ASSERT_NE(tok, nullptr);
        tok = (pp_token*)list_next(tok); /* <name> */
        ASSERT_NE(tok, nullptr);
        EXPECT_EQ(tok->type, pp_hdr_name);
        EXPECT_EQ(pp_token_valcmp(tok, "<name>"), 0);

        pp_free(&pp);
        fs_free(&fs);
        remove(file_name);
}


GTEST_API_ int main(int argc, char **argv) {
  printf("Running PP tests from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}