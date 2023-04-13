#include <stdbool.h>
#include <preproc.h>

_Bool pr_remove_newline_esc(struct pru_iter begin, struct pru_iter end)
{
        begin.snip_it.curr_sn->content[0] = 'a';
        return end.snip_it.position == 0;
}