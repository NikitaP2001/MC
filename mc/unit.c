#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <unit.h>
#include <list.h>


_Bool unit_from_file(struct tr_unit *init_unit, const char *file_name)
{
        _Bool status = false;
        init_unit->snippets = snippet_create(file_name);
        if (init_unit->snippets != NULL) {
                snippet_read_file(init_unit->snippets);
                status = true;
        }
        return status;
}

struct unit_reader *unit_get_reader(struct tr_unit *unit)
{
        struct unit_reader *reader = malloc(sizeof(struct unit_reader));
        reader->read_pos = 0;
        reader->curr_line = 0;
        reader->curr_snippet = unit->snippets;
        return reader;
}


void unit_free(struct tr_unit *unit)
{
        dlist_free(unit->snippets, (void (*)(void*))snippet_destroy);
}
