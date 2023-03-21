#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <check.h>

#include <unit.h>
#include <reader.h>
#include <mc.h>

#define MODULE_NAME "Reader"
#define CORE_NAME "Core"

#define FSIZE_MIN 0x100
#define FSIZE_MAX 0x1000
#define STR_INT_SIZE 10


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


static char *get_file_name(int file_number)
{
        char *name = NULL;
        const char *name_base = "testfile";
        char suffix[STR_INT_SIZE] = {0};

        sprintf(suffix, "%d", file_number);
        name = malloc(sizeof(name_base) + STR_INT_SIZE);
        strcpy(name, name_base);
        strcat(name, suffix);
        return name;
}

/* file_names - empty array of size @file_count * sizeof(char*) */
static void get_file_names(char **file_names, int file_count)
{
        for (int i = 0; i < file_count; i++)
                file_names[i] = get_file_name(i); 
}

/* generated @content array will be zero terminated */
static int gen_file_content(char *name, char **content)
{
        size_t fsize = 0;
        FILE *fp = fopen(name, "w");
        if (fp != NULL) {
                fsize = get_rand(FSIZE_MIN, FSIZE_MAX);
                char *new_content = calloc(fsize + 1, sizeof(char));
                for (size_t i = 0; i < fsize; i++)
                        new_content[i] = get_rand_chr();
                if (fsize != fwrite(new_content, sizeof(char), fsize, fp))
                        fsize = 0;
                *content = new_content;
                fclose(fp);
        }
        return fsize;
}

static _Bool remove_files(char **file_names, int file_count)
{
        _Bool status = true;
        for (int i = 0; i < file_count; i++) {
                if (remove(file_names[i]) != 0)
                        status = false;
                free(file_names[i]);
        }
        return status;        
}

START_TEST(test_reader_file_name)
{


}
END_TEST

START_TEST(test_reader_line_number)
{

        


}
END_TEST

START_TEST(test_reader_getc)
{
        const int count = 100;
        char *file_names[count];
        char *content[count];
        struct tr_unit unit = {0};
        struct unit_reader *reader = NULL;

        get_file_names(file_names, count);

        for (int i = 0; i < count; i++)
                ck_assert(gen_file_content(file_names[i], &content[i]));

        enum mc_status status = unit_from_file(&unit, file_names[0]);
        ck_assert(MC_SUCC(status));
       
        struct snippet *head = unit.first_snippet;
        for (int i = 1; i < count; i++) {
                struct snippet *new_sn = snippet_create(file_names[i]);
                ck_assert_ptr_ne(new_sn, NULL);

                status = snippet_read_file(new_sn);
                ck_assert(MC_SUCC(status));

                snippet_append(head, new_sn);
                head = new_sn;
        }

        reader = unit_get_reader(&unit);
        ck_assert_ptr_ne(reader, NULL);

        for (int i = 0; i < count; i++) {
                char_t chr = 0;
                for (int fpos = 0; content[i][fpos] != '\0'; fpos++) {
                        chr = reader_getc(reader);
                        ck_assert_int_eq(chr, content[i][fpos]);
                }
                chr = reader_getc(reader);
                if (i + 1 < count)
                        ck_assert_int_eq(chr, '\n');
                else
                        ck_assert_int_eq(chr, EOF);
                free(content[i]);
        }

        reader_destroy(reader);
        unit_destroy(&unit);
        remove_files(file_names, count);
}

START_TEST(test_reader_getc_empty)
{
        struct tr_unit unit = {0};
        struct unit_reader *reader = NULL;
        const char *file_name = "testfile.txt";

        FILE *fp = fopen(file_name, "wb");
        ck_assert_ptr_ne(fp, NULL);
        fclose(fp);

        enum mc_status status = unit_from_file(&unit, file_name);
        if (!MC_SUCC(status)) {
                puts(strerror(errno));
                puts(mc_str_status(status));
                ck_abort();
        }
        reader = unit_get_reader(&unit);
        ck_assert_ptr_ne(reader, NULL);

        char_t chr = reader_getc(reader);
        ck_assert_int_eq(chr, EOF);
        ck_assert_int_eq(reader_line_number(reader), 0);
        ck_assert_str_eq(reader_file_name(reader), file_name);

        reader_destroy(reader);
        unit_destroy(&unit);
        ck_assert_int_eq(remove(file_name), 0);
}
END_TEST

Suite *reader_suite(void)
{
        Suite *s = NULL;
        TCase *tc_core = NULL;
        s = suite_create(MODULE_NAME);
        tc_core = tcase_create(CORE_NAME);

        tcase_add_test(tc_core, test_reader_getc_empty);
        tcase_add_test(tc_core, test_reader_getc);
        tcase_add_test(tc_core, test_reader_file_name);
        tcase_add_test(tc_core, test_reader_line_number);
        suite_add_tcase(s, tc_core);
        return s;
}


void init_rand()
{
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        srand(ts.tv_nsec);
}


int main(void)
{
        int number_failed;
        Suite *s;
        SRunner *sr;

        init_rand();
        s = reader_suite();
        sr = srunner_create(s);
        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


