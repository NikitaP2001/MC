#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <prunit.h>
#include <reader.h>
#include <mc.h>
#include <tools.h>

#include "check_reader.h"
#include "rndfile_gen.h"

#define MODULE_NAME "Reader"
#define CORE_NAME "Core"

static size_t test_power = 1;


static struct txtgen_conf reader_test_init(size_t fcount)
{
        struct txtgen_conf conf = { .power = test_power };
        generator_init(&conf, fcount);
        return conf;
}


START_TEST(init)
{
        const int fcount = 1;
        struct pr_unit unit = {0};
        struct unit_reader *reader = NULL;
        struct txtgen_conf conf = reader_test_init(fcount);
        
        enum mc_status status = unit_from_files(&unit, conf.file_names, fcount);
        ck_assert(MC_SUCC(status));

        reader = reader_create(unit_begin(&unit), unit_end());
        ck_assert_ptr_ne(reader, NULL);

        reader_destroy(reader);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST


START_TEST(getc_newline)
{
        const int fcount = test_power * 3;
        struct txtgen_conf conf = reader_test_init(fcount);
        struct pr_unit unit = {0};
        struct unit_reader *reader = NULL;
        
        unit_from_files(&unit, conf.file_names, fcount);
        reader = reader_create(unit_begin(&unit), unit_end());

        for (int i = 0; i < fcount; i++) {
                char_t chr = 0;
                for (int fpos = 0; conf.content[i][fpos] != '\0'; fpos++)
                        chr = reader_getc(reader);

                chr = reader_getc(reader);
                if (i + 1 < fcount)
                        ck_assert_int_eq(chr, '\n');
        }

        reader_destroy(reader);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(getc_eof)
{
        const int fcount = test_power * 3;
        struct txtgen_conf conf = reader_test_init(fcount);
        struct pr_unit unit = {0};
        struct unit_reader *reader = NULL;
        
        unit_from_files(&unit, conf.file_names, fcount);
        reader = reader_create(unit_begin(&unit), unit_end());

        for (int i = 0; i < fcount; i++) {
                char_t chr = 0;
                for (int fpos = 0; conf.content[i][fpos] != '\0'; fpos++)
                        chr = reader_getc(reader);

                chr = reader_getc(reader);
                if (i + 1 == fcount)
                        ck_assert_int_eq(chr, EOF);
        }

        reader_destroy(reader);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(getc_read)
{
        const int fcount = test_power * 3;
        struct txtgen_conf conf = reader_test_init(fcount);
        struct pr_unit unit = {0};
        struct unit_reader *reader = NULL;

        unit_from_files(&unit, conf.file_names, fcount);
        reader = reader_create(unit_begin(&unit), unit_end());

        for (int i = 0; i < fcount; i++) {
                char_t chr = 0;
                for (int fpos = 0; conf.content[i][fpos] != '\0'; fpos++) {
                        chr = reader_getc(reader);
                        ck_assert_int_eq(chr, conf.content[i][fpos]);
                }
                chr = reader_getc(reader);
        }

        reader_destroy(reader);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(file_name)
{
        const int fcount = test_power * 2;
        struct txtgen_conf conf = reader_test_init(fcount);
        struct pr_unit unit = {0};
        struct unit_reader *reader = NULL;

        unit_from_files(&unit, conf.file_names, fcount);
        reader = reader_create(unit_begin(&unit), unit_end());
        for (int i = 0; i < fcount; i++) {
                ck_assert_str_eq(reader_file_name(reader), conf.file_names[i]);
                for (int fpos = 0; conf.content[i][fpos] != '\0'; fpos++)
                        reader_getc(reader);
                reader_getc(reader);
        }

        reader_destroy(reader);
        unit_destroy(&unit);
        generator_free(&conf);
}
END_TEST

START_TEST(file_line_number)
{
        const int fcount = test_power * 2;
        struct txtgen_conf conf = reader_test_init(fcount);
        struct pr_unit unit = {0};
        struct unit_reader *reader = NULL;

        unit_from_files(&unit, conf.file_names, fcount);
        reader = reader_create(unit_begin(&unit), unit_end());

        size_t lineNumber = 0;
        for (int i = 0; i < fcount; i++) {
                for (int fpos = 0; conf.content[i][fpos] != '\0'; fpos++) {
                        if (conf.content[i][fpos] == '\n')
                                lineNumber += 1;
                        reader_getc(reader);
                        ck_assert_int_eq(reader_line_number(reader), lineNumber);
                }
                lineNumber = 0;
                reader_getc(reader);
        }

        reader_destroy(reader);
        unit_destroy(&unit);
        generator_free(&conf);

}
END_TEST


Suite *reader_suite(size_t power)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        test_power = power;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);

        tcase_add_test(tc_core, init);
        tcase_add_test(tc_core, getc_read);
        tcase_add_test(tc_core, getc_eof);
        tcase_add_test(tc_core, getc_newline);
        tcase_add_test(tc_core, file_name);
        tcase_add_test(tc_core, file_line_number);
        suite_add_tcase(s, tc_core);
        return s;
}





