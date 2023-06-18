#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>

#include <check.h>

#include <source_provider.h>
#include <tools.h>
#include "rndfile.h"

#define MODULE_NAME "Prunit"
#define CORE_NAME "Core"

static size_t test_power = 1;

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
                        remove(name);
                free(name);
        }
}

START_TEST(read_unk_charset)
{
        char *file_name = "src.ctt";
        char *test_text = "This is - \x02 an unknown char!\n";
        char *exp_test = "This is -  an unknown char!\n";
        write_file(file_name, test_text, sizeof(test_text));
        struct source_provider pr = {0};
        provider_init(&pr);
        provider_add_local(&pr, "./");
        struct source_file *src = provider_get_local(&pr, file_name);
        char *srctext = source_read(src);
        ck_assert_str_eq(exp_test, srctext);
        ck_assert_int_eq(provider_lasterr(&pr), MC_UNKNOWN_CHAR);
        free(srctext);
        provider_free(&pr);
        remove(file_name);
}
END_TEST

START_TEST(read_no_end_newline)
{
        char *file_name = "src.ctt";
        char *test_text = "No newline \nthere";
        char *exp_test = "No newline \nthere\n";
        write_file(file_name, test_text, sizeof(test_text));
        struct source_provider pr = {0};
        provider_init(&pr);
        provider_add_local(&pr, "./");
        struct source_file *src = provider_get_local(&pr, file_name);
        char *srctext = source_read(src);
        ck_assert_str_eq(exp_test, srctext);
        free(srctext);
        provider_free(&pr);
        remove(file_name);
}
END_TEST

START_TEST(read_end_newline)
{
        char *file_name = "src.ctt";
        char *test_text = "There is a newline \nthere\n";
        char *exp_test = "There is a newline \nthere\n";
        write_file(file_name, test_text, sizeof(test_text));
        struct source_provider pr = {0};
        provider_init(&pr);
        provider_add_local(&pr, "./");
        struct source_file *src = provider_get_local(&pr, file_name);
        char *srctext = source_read(src);
        ck_assert_str_eq(exp_test, srctext);
        free(srctext);
        provider_free(&pr);
        remove(file_name);
}
END_TEST

START_TEST(read_empty)
{
        char *file_name = "src.ctt";
        char *test_text = "";
        char *exp_test = "";
        write_file(file_name, test_text, sizeof(test_text));
        struct source_provider pr = {0};
        provider_init(&pr);
        provider_add_local(&pr, "./");
        struct source_file *src = provider_get_local(&pr, file_name);
        char *srctext = source_read(src);
        ck_assert_str_eq(exp_test, srctext);
        free(srctext);
        provider_free(&pr);
        remove(file_name);
}
END_TEST

START_TEST(add_dir)
{
        create_dirs(test_power);
        struct source_provider pr = {0};
        provider_init(&pr);
        for (size_t i = 0; i < test_power; i++) {
                char *name = generate_name(i, DIRN_PREFIX);
                _Bool res = provider_add_local(&pr, name);
                ck_assert(res == false);
                ck_assert(MC_SUCC(provider_lasterr(&pr)));
                res = provider_add_global(&pr, name);
                ck_assert(res == false);
                ck_assert(MC_SUCC(provider_lasterr(&pr)));
                free(name);
        }
        provider_free(&pr);
        rm_dirs(test_power);
}
END_TEST

START_TEST(add_dir_invalid)
{
        struct source_provider pr = {0};
        const char *dirname = "/,,,/..";
        provider_init(&pr);
        _Bool res = provider_add_local(&pr, dirname);
        ck_assert(res == false);
        ck_assert_int_eq(provider_lasterr(&pr), MC_INVALID_PATH);
        res = provider_add_global(&pr, dirname);
        ck_assert(res == false);
        ck_assert_int_eq(provider_lasterr(&pr), MC_INVALID_PATH);
        provider_free(&pr);
}
END_TEST

START_TEST(get_file_local)
{
        struct source_provider pr = {0};
        struct txtgen_conf conf = {0};
        generator_init(&conf, 1);
        provider_init(&pr);
        provider_add_local(&pr, "./");
        struct source_file *found = provider_get_global(&pr, conf.file_names[0]);
        ck_assert_ptr_null(found);
        ck_assert_int_eq(provider_lasterr(&pr), MC_OPEN_FILE_FAIL);
        found = provider_get_local(&pr, conf.file_names[0]);
        ck_assert_ptr_nonnull(found);
        provider_free(&pr);
        generator_free(&conf);
}
END_TEST

START_TEST(get_file_global)
{
        struct source_provider pr = {0};
        struct txtgen_conf conf = {0};
        generator_init(&conf, 1);
        provider_init(&pr);
        provider_add_global(&pr, "./");
        struct source_file *found = provider_get_global(&pr, conf.file_names[0]);
        ck_assert_ptr_null(found);
        ck_assert_int_eq(provider_lasterr(&pr), MC_OPEN_FILE_FAIL);
        found = provider_get_local(&pr, conf.file_names[0]);
        ck_assert_ptr_nonnull(found);
        provider_free(&pr);
        generator_free(&conf);
}
END_TEST

START_TEST(get_file_samename_local)
{
        create_dirs(2);
        struct source_provider pr = {0};
        provider_init(&pr);

        char *dir1 = generate_name(0, DIRN_PREFIX);
        provider_add_local(&pr, dir1);

        char *dir2 = generate_name(1, DIRN_PREFIX);
        provider_add_local(&pr, dir2);

        struct txtgen_conf conf1 = { .dir_path = dir1 };
        generator_init(&conf1, 1);
        struct txtgen_conf conf2 = { .dir_path = dir2 };
        generator_init(&conf2, 1);

        struct source_file *found = provider_get_local(&pr, conf1.file_names[0]);
        ck_assert_ptr_nonnull(found);
        provider_free(&pr);

        generator_free(&conf2);
        generator_free(&conf1);
        free(dir2);
        free(dir1);
        rm_dirs(test_power);
}
END_TEST

START_TEST(get_file_samename_global)
{
        create_dirs(2);
        struct source_provider pr = {0};
        provider_init(&pr);

        char *dir1 = generate_name(0, DIRN_PREFIX);
        provider_add_global(&pr, dir1);

        char *dir2 = generate_name(1, DIRN_PREFIX);
        provider_add_global(&pr, dir2);

        struct txtgen_conf conf1 = { .dir_path = dir1 };
        generator_init(&conf1, 1);
        struct txtgen_conf conf2 = { .dir_path = dir2 };
        generator_init(&conf2, 1);

        struct source_file *found = provider_get_local(&pr, conf1.file_names[0]);
        ck_assert_ptr_nonnull(found);
        provider_free(&pr);

        generator_free(&conf2);
        generator_free(&conf1);
        free(dir2);
        free(dir1);
        rm_dirs(test_power);
}
END_TEST


Suite *srcprov_suite(size_t power)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);
        test_power = power;

        tcase_add_test(tc_core, read_unk_charset);
        tcase_add_test(tc_core, read_no_end_newline);
        tcase_add_test(tc_core, read_end_newline);
        tcase_add_test(tc_core, read_empty);
        tcase_add_test(tc_core, add_dir);
        tcase_add_test(tc_core, add_dir_invalid);
        tcase_add_test(tc_core, get_file_local);
        tcase_add_test(tc_core, get_file_global);
        tcase_add_test(tc_core, get_file_samename_local);
        tcase_add_test(tc_core, get_file_samename_global);
        
        suite_add_tcase(s, tc_core);
        return s;
}