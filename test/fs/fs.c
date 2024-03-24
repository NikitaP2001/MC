#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/stat.h>

#include <fs.h>
#include <test_common.h>

#define DIR_NAME "testdir"

static inline 
void create_dir(const char *name)
{
#ifdef _WIN32
        mkdir(name);
#else /* _WIN32 */
        mkdir(name, 0700);
#endif /* _WIN32 */
}

static inline 
void rm_dir(const char *name)
{
        rmdir(name);
}

TEST_CASE(fs, read_unk_charset)
{
        const char file_name[] = "src.ctt";
        const char test_text[] = "This is - \x02 an unknown char!\n";
        write_file(file_name, test_text, LEN_OF(test_text));
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = fs_file_read(src);
        ASSERT_EQ(srctext, NULL);
        UNUSED(srctext);
        ASSERT_EQ(fs_file_lasterr(src), MC_UNKNOWN_CHAR);
        fs_release_file(src);
        fs_free(&pr);
        remove(file_name);
}

TEST_CASE(fs, read_no_end_newline)
{
        const char *file_name = "src.ctt";
        const char test_text[] = "No newline \nthere";
        const char exp_test[] = "No newline \nthere\n";
        write_file(file_name, test_text, LEN_OF(test_text));
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = fs_file_read(src);
        ASSERT_EQ(strcmp(exp_test, srctext), 0);
        fs_release_file(src);
        fs_free(&pr);
        remove(file_name);
}

TEST_CASE(fs, read_end_newline)
{
        const char *file_name = "src.ctt";
        const char test_text[] = "There is a newline \nthere\n";
        const char exp_test[] = "There is a newline \nthere\n";
        write_file(file_name, test_text, LEN_OF(test_text));
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = fs_file_read(src);
        ASSERT_EQ(strcmp(exp_test, srctext), 0);
        fs_release_file(src);
        fs_free(&pr);
        remove(file_name);
}

TEST_CASE(fs, read_empty)
{
        const char *file_name = "src.ctt";
        const char test_text[] = "";
        const char exp_test[] = "";
        write_file(file_name, test_text, LEN_OF(test_text));
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = fs_file_read(src);
        ASSERT_EQ(strcmp(exp_test, srctext), 0);
        fs_release_file(src);
        fs_free(&pr);
        remove(file_name);
}

TEST_CASE(fs, add_dir)
{
        create_dir(DIR_NAME);
        struct filesys pr = {0};
        fs_init(&pr);

        fs_add_local(&pr, DIR_NAME);
        ASSERT_TRUE(MC_SUCC(fs_lasterr(&pr)));
        fs_add_global(&pr, DIR_NAME);
        ASSERT_TRUE(MC_SUCC(fs_lasterr(&pr)));

        fs_free(&pr);
        rm_dir(DIR_NAME);
}

TEST_CASE(fs, get_file_local)
{
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        write_file(TFILE_NAME, "", sizeof(""));
        struct fs_file *found = fs_get_global(&pr, "no_such_file.c");
        ASSERT_EQ(found, NULL);
        found = fs_get_local(&pr, TFILE_NAME);
        ASSERT_NE(found, NULL);
        fs_release_file(found);
        fs_free(&pr);
        remove(TFILE_NAME);
}

TEST_CASE(fs, two_local_dirs)
{
        struct filesys pr = {0};
        fs_init(&pr);
        write_file(TFILE_NAME, "", sizeof(""));
        fs_add_local(&pr, DIR_NAME);
        fs_add_local(&pr, "./");
        struct fs_file *found = fs_get_global(&pr, TFILE_NAME);
        ASSERT_EQ(found, NULL);
        found = fs_get_local(&pr, TFILE_NAME);
        ASSERT_NE(found, NULL);
        fs_release_file(found);
        fs_free(&pr);
        remove(TFILE_NAME);
}

TEST_CASE(fs, two_global_dirs)
{
        struct filesys pr = {0};
        fs_init(&pr);
        write_file(TFILE_NAME, "", sizeof(""));
        fs_add_global(&pr, DIR_NAME);
        fs_add_global(&pr, "./");
        struct fs_file *found = fs_get_global(&pr, TFILE_NAME);
        ASSERT_NE(found, NULL);
        fs_release_file(found);
        fs_free(&pr);
        remove(TFILE_NAME);
}

TEST_CASE(fs, get_file_global)
{
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_global(&pr, "./");
        write_file(TFILE_NAME, "", sizeof(""));
        struct fs_file *found = fs_get_global(&pr, TFILE_NAME);
        ASSERT_NE(found, NULL);
        fs_release_file(found);
        found = fs_get_local(&pr, TFILE_NAME);
        ASSERT_NE(found, NULL);
        fs_release_file(found);
        fs_free(&pr);
        remove(TFILE_NAME);
}

TEST_CASE(fs, get_file_notexist)
{
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, TFILE_NAME);
        ASSERT_EQ(src, NULL);
        fs_free(&pr);
}

int main()
{
        TEST_RUN(fs, read_unk_charset);
        TEST_RUN(fs, read_no_end_newline);
        TEST_RUN(fs, read_end_newline);
        TEST_RUN(fs, read_empty);
        TEST_RUN(fs, add_dir);
        TEST_RUN(fs, get_file_local);
        TEST_RUN(fs, two_local_dirs);
        TEST_RUN(fs, two_global_dirs);
        TEST_RUN(fs, get_file_global);
        TEST_RUN(fs, get_file_notexist);
}