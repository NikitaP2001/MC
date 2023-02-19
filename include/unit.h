#ifndef _UNIT_
#define _UNIT_

#include <reader.h>

struct tr_unit {
        
        struct snippet *snippets;
        
        
};

_Bool unit_from_file(const char *file_name, struct tr_unit *unit);

_Bool unit_expand_macro(struct tr_unit *unit);

_Bool unit_erase_comments(struct tr_unit *unit);

struct unit_reader *unit_get_reader(struct tr_unit *unit);

#endif /* _UNIT_ */
