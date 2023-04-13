#ifndef _UNIT_H_
#define _UNIT_H_

#include <snippet.h>

struct pr_unit {
        
        struct snippet *first_snippet;
        
        
};

/* TODO: remove that func, revrite tests */
enum mc_status unit_from_files(struct pr_unit *unit, char **file_names, 
                size_t file_count);

/* implement + write tests this will be new function */
//void pu_create(struct pr_unit *unit, struct source_provider *src_prov);

void unit_destroy(struct pr_unit *unit);

struct pru_iter {
        struct sn_iter snip_it;
};

/* The behavior is undefined if last is not reachable 
 * from first by (possibly repeatedly) incrementing first */
size_t pri_distance(struct pru_iter first, struct pru_iter second);

struct pru_iter pri_advance(struct pru_iter iter, size_t offset);

_Bool pri_cmp(struct pru_iter first, struct pru_iter second);

struct pru_iter unit_begin(struct pr_unit *unit);

struct pru_iter unit_end();


#endif /* _UNIT_H_ */
