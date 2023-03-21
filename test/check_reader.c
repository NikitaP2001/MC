#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <unit.h>
#include <reader.h>
#include <mc.h>

#include "check_reader.h"

struct test_conf {
        size_t min_fsize;
        size_t max_fsize;
        size_t fcount;
        char_t **content;
        char **file_names;
};

#define FSIZE_MIN 0x100
#define FSIZE_MAX 0x1000


static inline int get_rand(int min, int max)
{
        return rand() % max + min;
}


static char ascii_chars[] = "\t\n\v\f\r !\"#$%&'()*+,-./0123456789:;<=>?@ABCD \
                  EFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

static inline int get_rand_chr()
{
        int chr_pos = get_rand(0, sizeof(ascii_chars) - 1);
        return ascii_chars[chr_pos];
}


/* generated @content array will be zero terminated */
static void gen_file_content(struct test_conf *conf, size_t file_number)
{
        size_t fsize = 0;
        FILE *fp = fopen(conf->file_names[file_number], "w");
        ck_assert(fp != NULL);

        fsize = get_rand(conf->min_fsize, conf->max_fsize);

        char *new_content = calloc(fsize + 1, sizeof(char));
        for (size_t i = 0; i < fsize; i++)
                new_content[i] = get_rand_chr();
        conf->content[file_number] = new_content;
        fwrite(new_content, sizeof(char_t),  fsize + 1, fp);

        ck_assert(!ferror(fp));
        fclose(fp);
}


#define FILE_PREFIX "testfile"
#define STR_INT_SIZE 10

static char *generate_name(int file_number)
{
        char *name = NULL;
        const char *name_base = FILE_PREFIX;
        char suffix[STR_INT_SIZE] = {0};

        sprintf(suffix, "%d", file_number);
        name = malloc(sizeof(name_base) + STR_INT_SIZE);
        strcpy(name, name_base);
        strcat(name, suffix);
        return name;
}


START_TEST(test_file_name)
{


}
END_TEST

START_TEST(test_line_number)
{

        


}
END_TEST


static void reader_test_init(struct test_conf *conf, size_t file_count)
{
        conf->min_fsize = FSIZE_MIN;
        conf->max_fsize = FSIZE_MAX;
        conf->fcount = file_count;
        conf->content = calloc(file_count, sizeof(char_t*));
        conf->file_names = calloc(file_count, sizeof(char*));

        for (size_t i = 0; i < file_count; i++) {
                conf->file_names[i] = generate_name(i); 
                gen_file_content(conf, i);
        }
}


static void reader_test_free(struct test_conf *conf)
{

        for (size_t i = 0; i < conf->fcount; i++) {
                remove(conf->file_names[i]);
                free(conf->file_names[i]);
                free(conf->content[i]);
        }
        free(conf->file_names);
        free(conf->content);
}


START_TEST(test_init)
{
        const int fcount = 1;
        struct test_conf conf = {0};
        struct tr_unit unit = {0};
        struct unit_reader *reader = NULL;

        reader_test_init(&conf, fcount);
        
        enum mc_status status = unit_from_files(&unit, conf.file_names, fcount);
        ck_assert(MC_SUCC(status));

        reader = unit_get_reader(&unit);
        ck_assert_ptr_ne(reader, NULL);

        reader_destroy(reader);
        unit_destroy(&unit);
        reader_test_free(&conf);
}
END_TEST


START_TEST(test_getc_newline)
{
        const int fcount = 100;
        struct test_conf conf = {0};
        struct tr_unit unit = {0};
        struct unit_reader *reader = NULL;

        conf.min_fsize = 0;
        conf.max_fsize = 15;
        reader_test_init(&conf, fcount);
        
        unit_from_files(&unit, conf.file_names, fcount);
        reader = unit_get_reader(&unit);

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
        reader_test_free(&conf);
}
END_TEST

START_TEST(test_getc_eof)
{
        const int fcount = 100;
        struct test_conf conf = {0};
        struct tr_unit unit = {0};
        struct unit_reader *reader = NULL;

        conf.min_fsize = 0;
        conf.max_fsize = 15;
        reader_test_init(&conf, fcount);
        
        unit_from_files(&unit, conf.file_names, fcount);
        reader = unit_get_reader(&unit);

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
        reader_test_free(&conf);
}
END_TEST

START_TEST(test_getc_read)
{
        const int fcount = 100;
        struct test_conf conf = {0};
        struct tr_unit unit = {0};
        struct unit_reader *reader = NULL;

        reader_test_init(&conf, fcount);
        
        unit_from_files(&unit, conf.file_names, fcount);
        reader = unit_get_reader(&unit);

        for (int i = 0; i < fcount; i++) {
                char_t chr = 0;
                for (int fpos = 0; conf.content[i][fpos] != '\0'; fpos++) {
                        chr = reader_getc(reader);
                        if (chr != conf.content[i][fpos])
                                __builtin_trap();
                        ck_assert_int_eq(chr, conf.content[i][fpos]);
                }
                chr = reader_getc(reader);
        }

        reader_destroy(reader);
        unit_destroy(&unit);
        reader_test_free(&conf);
}
END_TEST


Suite *reader_suite(void)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);

        tcase_add_test(tc_core, test_init);
        tcase_add_test(tc_core, test_getc_read);
        tcase_add_test(tc_core, test_getc_eof);
        tcase_add_test(tc_core, test_getc_newline);
        tcase_add_test(tc_core, test_file_name);
        tcase_add_test(tc_core, test_line_number);
        suite_add_tcase(s, tc_core);
        return s;
}





