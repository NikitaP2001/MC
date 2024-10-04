#ifndef _LOOKAHEAD_H_
#define _LOOKAHEAD_H_
#include <parser.h>

static inline struct pt_node *
parser_lookahead_swap(struct parser *ps, struct pt_node *new)
{
        struct pt_node *old = ps->lookahead;
        ps->lookahead = new;
        return old;
}

static inline void
parser_lookahead_add(struct parser *ps, struct pt_node *child)
{
        pt_node_child_add(ps->lookahead, child);
}

static inline void
parser_lookahead_restore(struct parser *ps, struct pt_node *old)
{
        if (ps->lookahead != NULL)
                pt_node_destroy(ps->lookahead);
        ps->lookahead = old;
}

static inline struct pt_node *
parser_lookahead_pull(struct parser *ps, enum parser_symbol type)
{
        struct pt_node *lk = ps->lookahead;
        if (lk != NULL && lk->sym == type)
                ps->lookahead = NULL;
        else
                lk = NULL;
        return lk;
}

static inline void
parser_lookahead_set_type(struct parser *ps, enum parser_symbol type)
{
        struct pt_node *lk = ps->lookahead;
        lk->sym = type;
}

#endif /* _LOOKAHEAD_H_ */