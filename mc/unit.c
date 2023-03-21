#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <unit.h>
#include <list.h>
#include <mc.h>


enum mc_status unit_from_file(struct tr_unit *init_unit, const char *file_name)
{
        init_unit->first_snippet = snippet_create(file_name);
        if (init_unit->first_snippet == NULL)
                return MC_MEMALLOC_FAIL;

        return snippet_read_file(init_unit->first_snippet);
}

struct unit_reader *unit_get_reader(struct tr_unit *unit)
{
        struct unit_reader *reader = malloc(sizeof(struct unit_reader));
        reader->read_pos = 0;
        reader->curr_line = 0;
        reader->curr_snippet = unit->first_snippet;
        return reader;
}


void unit_destroy(struct tr_unit *unit)
{
        void (*free_fn)(void*) = (void (*)(void*))snippet_destroy;
        dlist_destroy(unit->first_snippet, free_fn);
}
