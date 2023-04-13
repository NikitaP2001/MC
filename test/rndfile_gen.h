#ifndef _RNDFILE_GEN_H
#define _RNDFILE_GEN_H
#include <mc.h>

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
};

_Bool write_file(const char *file_name, const char *content, size_t length);

void generator_init(struct txtgen_conf *conf, size_t file_count);

void generator_free(struct txtgen_conf *conf);

#endif /* _RNDFILE_GEN_H */