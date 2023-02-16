#include <stdio.h>
#include <stdbool.h>

#include <unit.h>


int unit_from_file(const char *file_name, tr_unit *init_unit)
{
        int status = true;
        init_unit->snippets = snippet_create(file_name);
        if (init_unit->snippets == NULL)
                status = false;
        return status;
}
