#include <stdio.h>
#include <stdbool.h>

#include <unit.h>


_Bool unit_from_file(const char *file_name, struct tr_unit *init_unit)
{
        _Bool status = true;
        init_unit->snippets = snippet_create(file_name);
        if (init_unit->snippets == NULL)
                status = false;
        return status;
}
