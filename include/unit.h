#ifndef _UNIT_
#define _UNIT_

#include <reader.h>

typedef struct tr_unit {
        
        struct snippet *snippets;
        
        
};

int unit_from_file(const char *file_name, tr_unit *unit);

int unit_expand_macro(tr_unit *unit);

int unit_erase_comments(tr_unit *unit);

unit_reader *unit_get_reader(tr_unit *unit);

#endif /* _UNIT_ */
