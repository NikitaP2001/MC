#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <prunit.h>
#include <snippet.h>
#include <reader.h>
#include <mc.h>
#include <tools.h>

#include "check_snippet.h"
#include "rndfile_gen.h"
#include "ttool.h"

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

START_TEST(insert_begin)
{
        struct txtgen_conf conf = snippet_test_init(2);
        struct pr_unit unit1 = {0}, unit2 = {0};
        unit_from_files(&unit1, &conf.file_names[0], 1);
        unit_from_files(&unit2, &conf.file_names[1], 1);
        struct pru_iter it1 = unit_begin(&unit1);
        sni_insert(it1.snip_it, unit2.first_snippet);
        struct unit_reader *reader = reader_create(it1, unit_end(&unit2));
        for (int i = 0; conf.content[1][i] != '\0'; i++) {
                char_t c = reader_getc(reader);
                ck_assert_int_eq(c, conf.content[1][i]);
        }
        for (int i = 0; conf.content[0][i] != '\0'; i++) {
                char_t c = reader_getc(reader);
                ck_assert_int_eq(c, conf.content[0][i]);
        }       
        reader_destroy(reader);
        unit_free(&unit1);
        unit_free(&unit2);
        generator_free(&conf);
}
END_TEST

START_TEST(insert_middle)
{
        struct txtgen_conf conf = snippet_test_init(2);
        struct pr_unit unit1 = {0}, unit2 = {0};
        unit_from_files(&unit1, &conf.file_names[0], 1);
        unit_from_files(&unit2, &conf.file_names[1], 1);
        struct pru_iter it1 = unit_begin(&unit1);
        size_t middle = pri_distance(unit_begin(&unit1), unit_end(&unit1)) / 2;
        it1 = pri_advance(it1, middle);
        sni_insert(it1.snip_it, unit2.first_snippet);
        struct unit_reader *reader = reader_create(it1, unit_end(&unit2));
        size_t pos1 = 0;
        for ( ; pos1 < middle; pos1++) {
                char_t c = reader_getc(reader);
                ck_assert_int_eq(c, conf.content[0][pos1]);
        }
        for (int i = 0; conf.content[1][i] != '\0'; i++) {
                char_t c = reader_getc(reader);
                ck_assert_int_eq(c, conf.content[1][i]);
        }
        ck_assert_int_eq(reader_getc(reader), '\n');
        for ( ; conf.content[0][pos1] != '\0'; pos1++) {
                char_t c = reader_getc(reader);
                ck_assert_int_eq(c, conf.content[0][pos1]);
        }       
        reader_destroy(reader);
        unit_free(&unit1);
        unit_free(&unit2);
        generator_free(&conf);
}
END_TEST

START_TEST(advance)
{
        struct txtgen_conf conf = snippet_test_init(1);
        struct pr_unit unit = {0};
        unit_from_files(&unit, conf.file_names, conf.fcount);
        size_t clen = cont_len(conf.content[0]);

        struct sn_iter it = {0};
        for (size_t i = 0; i < clen; i++) {
                ck_assert_int_eq(it.position, i);
                it = sni_advance(it, i);
        }
        ck_assert(sni_cmp(it, snippet_end()));
        unit_free(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(begin)
{
        struct txtgen_conf conf = snippet_test_init(1);
        struct snippet *sn = snippet_create(conf.file_names[0]);
        snippet_read_file(sn);
        struct sn_iter beg = snippet_begin(sn);
        ck_assert_int_eq(beg.position, 0);
        ck_assert_ptr_eq(beg.curr_sn, sn);
        snippet_destroy(sn);
        generator_free(&conf);
}
END_TEST

START_TEST(end)
{
        struct sn_iter beg = snippet_end();
        ck_assert_ptr_eq(beg.curr_sn, NULL);
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
        tcase_add_test(tc_core, begin);
        tcase_add_test(tc_core, end);
        tcase_add_test(tc_core, advance);
        tcase_add_test(tc_core, insert_begin);
        tcase_add_test(tc_core, insert_middle);
        
        suite_add_tcase(s, tc_core);
        return s;
}