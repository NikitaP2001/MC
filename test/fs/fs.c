#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <sys/stat.h>

#include <fs.h>

#define DIR_NAME "testdir"
#define TFILE_NAME "file.cc"

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
static _Bool 
write_file(const char *file_name, const char *content, size_t length)
{
        _Bool result = false;
        FILE *fp = fopen(file_name, "wb");
        if (fp != NULL) {
                fwrite(content, sizeof(char), length, fp);

                result = ferror(fp) == 0;
                fclose(fp);
        }
        return result; 
}

static void fs_read_unk_charset()
{
        const char file_name[] = "src.ctt";
        const char test_text[] = "This is - \x02 an unknown char!\n";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        assert(srctext == NULL);
        assert(source_lasterr(src) == MC_UNKNOWN_CHAR);
        fs_free(&pr);
        remove(file_name);
}

static void fs_read_no_end_newline()
{
        const char *file_name = "src.ctt";
        const char test_text[] = "No newline \nthere";
        const char exp_test[] = "No newline \nthere\n";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        assert(strcmp(exp_test, srctext) == 0);
        fs_free(&pr);
        remove(file_name);
}

static void fs_read_end_newline()
{
        const char *file_name = "src.ctt";
        const char test_text[] = "There is a newline \nthere\n";
        const char exp_test[] = "There is a newline \nthere\n";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        assert(strcmp(exp_test, srctext) == 0);
        fs_free(&pr);
        remove(file_name);
}

static void fs_read_empty()
{
        const char *file_name = "src.ctt";
        const char test_text[] = "";
        const char exp_test[] = "";
        write_file(file_name, test_text, sizeof(test_text) - 1);
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, file_name);
        const char *srctext = source_read(src);
        assert(strcmp(exp_test, srctext) == 0);
        fs_free(&pr);
        remove(file_name);
}

static void fs_add_dir()
{
        create_dir(DIR_NAME);
        struct filesys pr = {0};
        fs_init(&pr);

        fs_add_local(&pr, DIR_NAME);
        assert(MC_SUCC(fs_lasterr(&pr)));
        fs_add_global(&pr, DIR_NAME);
        assert(MC_SUCC(fs_lasterr(&pr)));

        fs_free(&pr);
        rm_dir(DIR_NAME);
}

static void fs_get_file_local()
{
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        write_file(TFILE_NAME, "", sizeof(""));
        struct fs_file *found = fs_get_global(&pr, "no_such_file.c");
        assert(found == NULL);
        found = fs_get_local(&pr, TFILE_NAME);
        assert(found != NULL);
        fs_free(&pr);
        remove(TFILE_NAME);
}

static void fs_get_file_two_dirs()
{
        struct filesys pr = {0};
        fs_init(&pr);
        write_file(TFILE_NAME, "", sizeof(""));
        fs_add_local(&pr, DIR_NAME);
        fs_add_local(&pr, "./");
        struct fs_file *found = fs_get_global(&pr, TFILE_NAME);
        assert(found == NULL);
        found = fs_get_local(&pr, TFILE_NAME);
        assert(found != NULL);
        fs_free(&pr);
        remove(TFILE_NAME);
}

static void fs_get_file_global()
{
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_global(&pr, "./");
        write_file(TFILE_NAME, "", sizeof(""));
        struct fs_file *found = fs_get_global(&pr, TFILE_NAME);
        assert(found != NULL);
        found = fs_get_local(&pr, TFILE_NAME);
        assert(found != NULL);
        fs_free(&pr);
        remove(TFILE_NAME);
}

static void fs_get_file_notexist()
{
        struct filesys pr = {0};
        fs_init(&pr);
        fs_add_local(&pr, "./");
        struct fs_file *src = fs_get_local(&pr, TFILE_NAME);
        assert(src == NULL);
        fs_free(&pr);
}

int main()
{
        puts("FS TESTS STARTED");
        fs_read_unk_charset();
        fs_read_no_end_newline();
        fs_read_end_newline();
        fs_read_empty();
        fs_add_dir();
        fs_get_file_local();
        fs_get_file_two_dirs();
        fs_get_file_global();
        fs_get_file_notexist();
        puts("FS TESTS FINISHED OK");
}