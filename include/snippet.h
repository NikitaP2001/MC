#ifndef _SNIPPET_H_
#define _SNIPPET_H_
#include <stddef.h>

#include <list.h>
#include <mc.h>

/* snippet represents some part of specific source file */
struct snippet {

        struct dlist_head list;

        char *file_name;
        
        size_t line_number;

        char_t *content;
};

/* @return null in result of alocation error */
struct snippet *snippet_create(const char *file_name);

enum mc_status snippet_read_file(struct snippet *sn);

void snippet_destroy(struct snippet *sn);

struct sn_iter {
        struct snippet *curr_sn;
        size_t position;
};

struct sn_iter sni_insert(struct sn_iter split_pos, struct snippet *other);

struct sn_iter sni_advance(struct sn_iter iter, size_t offset);

_Bool sni_cmp(struct sn_iter first, struct sn_iter second);

struct sn_iter snippet_begin(struct snippet *sn);

struct sn_iter snippet_end();


#endif /* _SNIPPET_H_ */
