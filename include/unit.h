#ifndef _UNIT_H_
#define _UNIT_H_

#include <reader.h>

struct tr_unit {
        
        struct snippet *first_snippet;
        
        
};

enum mc_status unit_from_files(struct tr_unit *unit, char **file_names, 
                size_t file_count);

_Bool unit_expand_macro(struct tr_unit *unit);

_Bool unit_erase_comments(struct tr_unit *unit);

struct unit_reader *unit_get_reader(struct tr_unit *unit);

void unit_destroy(struct tr_unit *unit);

#endif /* _UNIT_H_ */
