#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <preproc.h>
#include <prunit.h>
#include <reader.h>
#include "ttool.h"
#include "check_preproc.h"
#include "rndfile_gen.h"

#define MODULE_NAME "Preproc"
#define CORE_NAME "Core"

static size_t test_power = 1;

static char *read_iter(struct pru_iter begin, struct pru_iter end)
{
        size_t dist = pri_distance(begin, end);
        char *result = malloc(dist);
        struct unit_reader *reader = reader_create(begin, end);
        for (size_t i = 0; i < dist; i++)
                result[i] = reader_getc(reader);
        return result;
}

START_TEST(escape_newline)
{
        char *test_text = "int a = 3; \\\n" \
        "/*this is \\comment*/ \\\n" \
        "int b = 4;\\";
        char *expected = "int a = 3; /*this is \\comment*/ int b = 4;\\";
        char *file_name = "src.c";
        write_file(file_name, test_text, sizeof(test_text));
        struct pr_unit unit = {0};
        unit_from_files(&unit, &file_name, 1);
        struct pru_iter it = unit_begin(&unit);
        struct pru_iter itend = unit_end(&unit);
        pr_remove_newline_esc(it, itend);
        char *result = read_iter(it, itend);
        ck_assert(wordcmp(result, expected));
        remove(file_name);
        free(result);
}
END_TEST

Suite *preproc_suite(size_t power)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        test_power = power;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);

        tcase_add_test(tc_core, escape_newline);
        
        suite_add_tcase(s, tc_core);
        return s;
}