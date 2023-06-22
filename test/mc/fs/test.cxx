#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

#include <gtest/gtest.h>

extern "C" {
        #include <rndfile.h>
        #include <fs.h>
        #include <tools.h>
}

#define DIRN_PREFIX "testdir"

void create_dirs(size_t count)
{
        for (size_t i = 0; i < count; i++) {
                char *name = generate_name(i, DIRN_PREFIX);
                if (!fs_isdir(name))
                        mkdir(name);
                free(name);
        }
}

void rm_dirs(size_t count)
{
        for (size_t i = 0; i < count; i++) {
                char *name = generate_name(i, DIRN_PREFIX);
                if (fs_isdir(name))
                        rmdir(name);
                free(name);
        }
}

TEST(fs_test, read_unk_charset)
{
        const char file_name[] = "src.ctt";
        const char test_text[] = "This is - \x02 an unknown char!\n";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        ASSERT_EQ(srctext, nullptr);
        ASSERT_EQ(source_lasterr(src), MC_UNKNOWN_CHAR);
        fs_free(&pr);
        remove(file_name);
}

TEST(fs_test, read_no_end_newline)
{
        const char *file_name = "src.ctt";
        const char test_text[] = "No newline \nthere";
        const char exp_test[] = "No newline \nthere\n";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        ASSERT_STREQ(exp_test, srctext);
        fs_free(&pr);
        remove(file_name);
}

TEST(fs_test, read_end_newline)
{
        const char *file_name = "src.ctt";
        const char test_text[] = "There is a newline \nthere\n";
        const char exp_test[] = "There is a newline \nthere\n";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        ASSERT_STREQ(exp_test, srctext);
        fs_free(&pr);
        remove(file_name);
}

TEST(fs_test, read_empty)
{
        const char *file_name = "src.ctt";
        const char test_text[] = "";
        const char exp_test[] = "";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        ASSERT_STREQ(exp_test, srctext);
        fs_free(&pr);
        remove(file_name);
}

TEST(fs_test, add_dir)
{
        create_dirs(1);
        struct filesys pr = {};
        fs_init(&pr);

        char *name = generate_name(0, DIRN_PREFIX);
        fs_add_local(&pr, name);
        ASSERT_TRUE(MC_SUCC(fs_lasterr(&pr)));
        fs_add_global(&pr, name);
        EXPECT_TRUE(MC_SUCC(fs_lasterr(&pr)));
        free(name);

        fs_free(&pr);
        rm_dirs(1);
}

TEST(fs_test, get_file_local)
{
        struct filesys pr = {};
        struct txtgen_conf conf = {};
        generator_init(&conf, 1);
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *found = fs_get_global(&pr, conf.file_names[0]);
        EXPECT_EQ(found, nullptr);
        found = fs_get_local(&pr, conf.file_names[0]);
        EXPECT_NE(found, nullptr);
        fs_free(&pr);
        generator_free(&conf);
}

TEST(fs_test, get_file_global)
{
        struct filesys pr = {};
        struct txtgen_conf conf = {};
        generator_init(&conf, 1);
        fs_init(&pr);
        fs_add_global(&pr, "./");
        struct fs_file *found = fs_get_global(&pr, conf.file_names[0]);
        EXPECT_NE(found, nullptr);
        found = fs_get_local(&pr, conf.file_names[0]);
        EXPECT_NE(found, nullptr);
        fs_free(&pr);
        generator_free(&conf);
}

TEST(fs_test, get_file_notexist)
{
        const char *file_name = "src.ctt";
        struct filesys pr = {};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        EXPECT_EQ(src, nullptr);
        fs_free(&pr);
}

GTEST_API_ int main(int argc, char **argv) {
  printf("Running FS tests from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}