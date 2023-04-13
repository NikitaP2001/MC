#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <check.h>

#include <mc.h>

#include "rndfile_gen.h"

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

_Bool write_file(const char *file_name, const char *content, size_t length)
{
        _Bool result = false;
        FILE *fp = fopen(file_name, "w");
        if (fp != NULL) {
                fwrite(content, sizeof(char_t), length, fp);

                result = ferror(fp) == 0;
                fclose(fp);
        }
        return result; 
}

/* generated @content array will be zero terminated */
static void gen_file_content(struct txtgen_conf *conf, size_t file_number)
{
        size_t fsize = 0;
        fsize = get_rand(conf->min_fsize, conf->max_fsize);
        const char *file_name = conf->file_names[file_number];

        char *new_content = calloc(fsize + 1, sizeof(char));
        for (size_t i = 0; i < fsize; i++)
                new_content[i] = get_rand_chr();
        conf->content[file_number] = new_content;
        ck_assert(write_file(file_name, new_content, fsize + 1));
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

void generator_init(struct txtgen_conf *conf, size_t file_count)
{
        conf->power = (conf->power == 0) ? SIZE_POWER : conf->power;
        conf->min_fsize = FSIZE_MIN * conf->power;
        conf->max_fsize = FSIZE_MAX * conf->power;
        conf->fcount = file_count;
        conf->content = calloc(file_count, sizeof(char_t*));
        conf->file_names = calloc(file_count, sizeof(char*));

        for (size_t i = 0; i < file_count; i++) {
                conf->file_names[i] = generate_name(i); 
                gen_file_content(conf, i);
        }
}


void generator_free(struct txtgen_conf *conf)
{

        for (size_t i = 0; i < conf->fcount; i++) {
                remove(conf->file_names[i]);
                free(conf->file_names[i]);
                free(conf->content[i]);
        }
        free(conf->file_names);
        free(conf->content);
}