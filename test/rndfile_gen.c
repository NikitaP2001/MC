#include <stdlib.h>
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


/* generated @content array will be zero terminated */
static void gen_file_content(struct txtgen_conf *conf, size_t file_number)
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