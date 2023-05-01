#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <prunit.h>
#include <snippet.h>
#include <list.h>
#include <mc.h>


static enum mc_status unit_from_file(struct pr_unit *init_unit, const char *file_name)
{
        init_unit->first_snippet = snippet_create(file_name);
        if (init_unit->first_snippet == NULL)
                return MC_MEMALLOC_FAIL;

        return snippet_read_file(init_unit->first_snippet);
}


enum mc_status unit_from_files(struct pr_unit *unit, char **file_names, 
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
                        unit_free(unit);
                        return MC_FAIL;
                }
                struct sn_iter itend = snippet_begin(head);
                sni_insert(itend, new_sn);
                head = new_sn;

                status = snippet_read_file(new_sn);
                if (!MC_SUCC(status)) {
                        unit_free(unit);
                        return status;
                }
        }

        return MC_OK;
}


void unit_free(struct pr_unit *unit)
{
        void (*free_fn)(void*) = (void (*)(void*))snippet_destroy;
        dlist_destroy(unit->first_snippet, free_fn);
}


size_t pri_distance(struct pru_iter first, struct pru_iter second)
{
        size_t result = 0;
        struct snippet *head = first.snip_it.curr_sn;
        struct snippet *last = second.snip_it.curr_sn;
        size_t dist_toend = strlen(head->content) - 1;
        /* if lists are seperate we choose to die */
        while (head != last) {
                result += dist_toend;
                head = dlist_next(head);
                dist_toend = (head != NULL) ? strlen(head->content) : 0;
        }
        result += second.snip_it.position - first.snip_it.position;
        return result;
        
}


struct pru_iter pri_advance(struct pru_iter iter, size_t offset)
{
        iter.snip_it = sni_advance(iter.snip_it, offset);
        return iter;
}

_Bool pri_cmp(struct pru_iter first, struct pru_iter second)
{
        if (first.snip_it.curr_sn == NULL || second.snip_it.curr_sn == NULL)
                return first.snip_it.curr_sn == second.snip_it.curr_sn;
        return first.snip_it.curr_sn == second.snip_it.curr_sn 
        && first.snip_it.position == second.snip_it.position;
}

struct pru_iter unit_begin(struct pr_unit *unit)
{
        struct pru_iter it_begin = {
                .snip_it = snippet_begin(unit->first_snippet)
        };
        return it_begin;
}

struct pru_iter unit_end()
{
        struct pru_iter it_end = {
                .snip_it = snippet_end(),
        };
        return it_end;
}