#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <unit.h>
#include <reader.h>
#include <mc.h>
#include <tools.h>

#include "check_snippet.h"

#define MODULE_NAME "Snippet"
#define CORE_NAME "Core"

static const char* file_name = "testfile0";

START_TEST(create_exist)
{
        FILE *fp = fopen(file_name, "w");
        fclose(fp);
        struct snippet *sn = snippet_create(file_name);
        ck_assert_ptr_nonnull(sn);
        snippet_destroy(sn);
        remove(file_name);
}
END_TEST

/*
START_TEST(sliced_file_name)
{
        const int fcount = 2;
        struct test_conf conf = {0};
        struct tr_unit unit = {0};
        struct unit_reader *reader = NULL;

        reader_test_init(&conf, fcount);
        
        unit_from_files(&unit, conf.file_names, 1);

        reader = unit_get_reader(&unit);

        FILE *ffirst = fopen(conf.file_names[0], "r");
        size_t sz_first = get_file_size(ffirst);
        fclose(ffirst);
        for (size_t pos = 0; pos < sz_first / 2; pos++)
                reader_getc(reader);
        ck_assert_str_eq(reader_file_name(reader), conf.file_names[0]);

        struct snippet *new_sn = snippet_create(conf.file_names[2]);
        struct snippet_iterator it = {0};
        reader_get_iter(reader, it);
        snippet_insert(it, new_sn);

        FILE *fsecond = fopen(conf.file_names[1], "r");
        size_t sz_second = get_file_size();
        fclose(fsecond);

        ck_assert_str_eq(reader_file_name(reader), conf.file_names[1]);
        reader_getc(reader);
        for (size_t pos = 0; pos < sz_second; pos++)
                reader_getc(reader);
        reader_getc(reader);
        ck_assert_str_eq(reader_file_name(reader), conf.file_names[0]);

        reader_destroy(reader);
        unit_destroy(&unit);
        reader_test_free(&conf);
}
END_TEST
*/

Suite *snippet_suite(void)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);

        tcase_add_test(tc_core, create_exist);
        
        suite_add_tcase(s, tc_core);
        return s;
}