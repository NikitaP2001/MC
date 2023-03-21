#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <unit.h>
#include <list.h>
#include <mc.h>


static enum mc_status unit_from_file(struct tr_unit *init_unit, const char *file_name)
{
        init_unit->first_snippet = snippet_create(file_name);
        if (init_unit->first_snippet == NULL)
                return MC_MEMALLOC_FAIL;

        return snippet_read_file(init_unit->first_snippet);
}


enum mc_status unit_from_files(struct tr_unit *unit, char **file_names, 
                size_t file_count)
{
        if (file_count == 0)
                return MC_INVALID_ARG;

        enum mc_status status = unit_from_file(unit, file_names[0]);
        if (!MC_SUCC(status))
                return status;

        struct snippet *head = unit->first_snippet;
        for (size_t i = 1; i < file_count; i++) {
                struct snippet *new_sn = snippet_create(file_names[i]);
                if (new_sn == NULL) {
                        unit_destroy(unit);
                        return MC_FAIL;
                }
                snippet_append(head, new_sn);
                head = new_sn;

                status = snippet_read_file(new_sn);
                if (!MC_SUCC(status)) {
                        unit_destroy(unit);
                        return status;
                }
        }

        return MC_OK;
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
