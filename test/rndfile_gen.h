#ifndef _RNDFILE_GEN_H
#define _RNDFILE_GEN_H
#include <mc.h>

#include "ttool.h"

#define FSIZE_MIN 0x10
#define FSIZE_MAX 0x100

#define SIZE_POWER 1

struct txtgen_conf {
        size_t power;
        size_t min_fsize;
        size_t max_fsize;
        size_t fcount;
        char_t **content;
        char **file_names;
        /* pass custom dir path if needed */
        char *dir_path;
};

void generator_init(struct txtgen_conf *conf, size_t file_count);

void generator_free(struct txtgen_conf *conf);

char *generate_name(int file_number, const char *prefix);

#endif /* _RNDFILE_GEN_H */