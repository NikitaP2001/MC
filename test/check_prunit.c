
#include <check.h>

#include <prunit.h>
#include <reader.h>
#include "check_prunit.h"
#include "rndfile_gen.h"
#include "ttool.h"

#define MODULE_NAME "Prunit"
#define CORE_NAME "Core"

static size_t test_power = 1;

static struct txtgen_conf prunit_test_init(size_t fcount)
{
        struct txtgen_conf conf = {0};
        generator_init(&conf, fcount);
        return conf;
}

START_TEST(advance_toend)
{
        struct txtgen_conf conf = prunit_test_init(1);
        struct pr_unit unit = {0};
        unit_from_files(&unit, conf.file_names, conf.fcount);
        size_t csize = cont_len(conf.content[0]);
        struct pru_iter it = unit_begin(&unit);
        it = pri_advance(it, csize);
        ck_assert(pri_cmp(it, unit_end()));
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(advance)
{
        struct txtgen_conf conf = prunit_test_init(test_power);
        struct pr_unit unit = {0};
        unit_from_files(&unit, conf.file_names, conf.fcount);

        struct pru_iter it = {0};
        for (size_t i = 0; i < conf.fcount - 1; i++) {
                size_t flen = cont_len(conf.content[i]);
                it = pri_advance(it, flen);
                struct unit_reader *rd = reader_create(it, unit_end());
                ck_assert_int_eq(reader_getc(rd), conf.content[i + 1][0]);
                reader_destroy(rd);

        }
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(distance_middle)
{
        struct txtgen_conf conf = prunit_test_init(3);
        struct pr_unit unit = {0};
        unit_from_files(&unit, conf.file_names, conf.fcount);

        size_t len1 = cont_len(conf.content[0]), 
        len2 = cont_len(conf.content[1]), len3 = cont_len(conf.content[2]);
        size_t off_beg = len1 / 2;
        size_t off_end = len3 / 2;

        struct pru_iter it_beg = pri_advance(unit_begin(&unit), off_beg);
        struct pru_iter it_end = pri_advance(unit_begin(&unit), 
        len1 + len2 + off_end);
        size_t dist = pri_distance(it_beg, it_end);

        /* + fcount - 1 newline added at the file edges */
        size_t exp_dist = len1 - off_beg - 1;
        exp_dist += len2;
        exp_dist += off_end;
        ck_assert_int_eq(exp_dist, dist);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(distance_end)
{
        struct txtgen_conf conf = prunit_test_init(3);
        struct pr_unit unit = {0};
        unit_from_files(&unit, conf.file_names, conf.fcount);
        size_t exp_dist = pri_distance(unit_begin(&unit), unit_end());
        size_t dist = 0;
        for (size_t i = 0; i < conf.fcount; i++)
                dist += cont_len(conf.content[i]);
        ck_assert_int_eq(dist, exp_dist);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(begin)
{
        struct txtgen_conf conf = prunit_test_init(2);
        struct pr_unit unit = {0};
        unit_from_files(&unit, conf.file_names, 2);
        struct pru_iter uit = unit_begin(&unit);
        ck_assert_ptr_eq(uit.snip_it.curr_sn, unit.first_snippet);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(end)
{
        struct pru_iter uit = unit_end();
        ck_assert_ptr_eq(uit.snip_it.curr_sn, NULL);
}
END_TEST


Suite *prunit_suite(size_t power)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        test_power = power;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);

        tcase_add_test(tc_core, begin);
        tcase_add_test(tc_core, end);
        tcase_add_test(tc_core, distance_end);
        tcase_add_test(tc_core, distance_middle);
        tcase_add_test(tc_core, advance);
        tcase_add_test(tc_core, advance_toend);
        
        suite_add_tcase(s, tc_core);
        return s;
}