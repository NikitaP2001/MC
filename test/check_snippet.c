#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <check.h>

#include <unit.h>
#include <reader.h>
#include <mc.h>
#include <tools.h>

#include "check_snippet.h"
#include "rndfile_gen.h"

#define MODULE_NAME "Snippet"
#define CORE_NAME "Core"

static const char* file_name = "testfile0";

static struct txtgen_conf snippet_test_init(size_t fcount)
{
        struct txtgen_conf conf = {0};
        generator_init(&conf, fcount);
        return conf;
}

START_TEST(create)
{
        struct snippet *sn = snippet_create(file_name);
        ck_assert_ptr_nonnull(sn);
        ck_assert_str_eq(sn->file_name, file_name);
        snippet_destroy(sn);
}
END_TEST

START_TEST(read_file)
{
        struct txtgen_conf conf = snippet_test_init(1);
        struct snippet *sn = snippet_create(conf.file_names[0]);
        enum mc_status result = snippet_read_file(sn);
        ck_assert_int_eq(result, MC_OK);
        ck_assert_ptr_nonnull(sn->content);
        snippet_destroy(sn);
        remove(file_name);
        generator_free(&conf);
}
END_TEST

START_TEST(read_file_notexist)
{
        struct snippet *sn = snippet_create(file_name);
        enum mc_status result = snippet_read_file(sn);
        ck_assert_int_eq(result, MC_OPEN_FILE_FAIL);
        snippet_destroy(sn);
}
END_TEST


Suite *snippet_suite(void)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);

        tcase_add_test(tc_core, create);
        tcase_add_test(tc_core, read_file);
        tcase_add_test(tc_core, read_file_notexist);
        
        suite_add_tcase(s, tc_core);
        return s;
}