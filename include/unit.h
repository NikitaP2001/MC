#ifndef _UNIT_H_
#define _UNIT_H_

#include <reader.h>

struct tr_unit {
        
        struct snippet *first_snippet;
        
        
};

enum mc_status unit_from_file(struct tr_unit *unit, const char *file_name);

_Bool unit_expand_macro(struct tr_unit *unit);

_Bool unit_erase_comments(struct tr_unit *unit);

struct unit_reader *unit_get_reader(struct tr_unit *unit);

void unit_destroy(struct tr_unit *unit);

#endif /* _UNIT_H_ */
