#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <check.h>

#include <mc.h>
#include <tools.h>
#include <rndfile.h>

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
        fsize = get_rand(conf->min_fsize, conf->max_fsize);
        const char *file_name = conf->file_names[file_number];

        char *new_content = calloc(fsize + 1, sizeof(char));
        for (size_t i = 0; i < fsize; i++)
                new_content[i] = get_rand_chr();
        conf->content[file_number] = new_content;
        ck_assert(write_file(file_name, new_content, fsize + 1));
}

#define STR_INT_SIZE 10

char *generate_name(int file_number, const char *prefix)
{
        char *name = NULL;
        char suffix[STR_INT_SIZE] = {0};

        sprintf(suffix, "%d", file_number);
        name = malloc(sizeof(prefix) + STR_INT_SIZE);
        strcpy(name, prefix);
        strcat(name, suffix);
        return name;
}

#define FILE_PREFIX "testfile"

static char *form_prefix(struct txtgen_conf *conf)
{
        char *prefix = NULL;
        if (conf->dir_path != 0) {
                size_t pref_size = strlen(conf->dir_path);
                pref_size += sizeof(FILE_PREFIX);
                prefix = malloc(pref_size);
                strcpy(prefix, conf->dir_path);
        } else
                prefix = calloc(sizeof(FILE_PREFIX), sizeof(char));
        strcat(prefix, FILE_PREFIX);
        return prefix;
}

void generator_init(struct txtgen_conf *conf, size_t file_count)
{
        conf->power = (conf->power == 0) ? SIZE_POWER : conf->power;
        conf->min_fsize = FSIZE_MIN * conf->power;
        conf->max_fsize = FSIZE_MAX * conf->power;
        conf->fcount = file_count;
        conf->dir_path = (conf->dir_path != NULL) ? fs_path(conf->dir_path) : NULL;
        conf->content = calloc(file_count, sizeof(char*));
        conf->file_names = calloc(file_count, sizeof(char*));

        char *prefix = form_prefix(conf);
        for (size_t i = 0; i < file_count; i++) {
                
                conf->file_names[i] = generate_name(i, prefix); 
                gen_file_content(conf, i);
        }
        free(prefix);
}


void generator_free(struct txtgen_conf *conf)
{
        for (size_t i = 0; i < conf->fcount; i++) {
                remove(conf->file_names[i]);
                free(conf->file_names[i]);
                free(conf->content[i]);
        }
        if (conf->dir_path != NULL)
                free(conf->dir_path);
        free(conf->file_names);
        free(conf->content);
}