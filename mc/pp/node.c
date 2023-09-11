#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <list.h>
#include <pp/node.h>

static _Bool
pp_node_group_insert(struct pp_node *group, struct pp_node *node);
static inline _Bool
pp_node_elif_group_insert(struct pp_node *elif_gr, struct pp_node *node);

struct pp_node*
pp_node_create(enum pp_node_type type)
{
        struct pp_node *node = (struct pp_node*)malloc(sizeof(struct pp_node));
        memset(node, 0, sizeof(struct pp_node));
        node->type = type;
        return node;
}

void
pp_node_destroy(struct pp_node *node)
{
        if (node->first_descendant != NULL)
                list_destroy(node->first_descendant, (list_free)pp_node_destroy);
        free(node);
}

void
pp_node_add_leaf(struct pp_node *node, struct pp_token *token)
{
        struct pp_node *desc = pp_node_create(pp_leaf);
        desc->leaf_tok = token;
        pp_node_add_descendant(node, desc);
}

void
pp_node_add_descendant(struct pp_node *node, struct pp_node *ch_node)
{
        if (node->last_descendant != NULL) {
                list_append(node->last_descendant, ch_node);
                node->last_descendant = ch_node;
        } else {
                node->first_descendant = node->last_descendant = ch_node;
        }
}

static inline _Bool
pp_node_has_descendant(const struct pp_node *node)
{
        return node->first_descendant != NULL;
}

static _Bool
pp_node_if_group_insert(struct pp_node *if_group, struct pp_node *node)
{
        _Bool status = false;
        if (if_group->last_descendant->type != pp_group) {
                struct pp_node *group = pp_node_create(pp_group);
                if ((status = pp_node_group_insert(group, node)))
                        pp_node_add_descendant(if_group, group);
                else
                        pp_node_destroy(group);
        } else
                status = pp_node_group_insert(if_group->last_descendant, node);
        return status;
}


static _Bool
pp_node_elif_groups_insert(struct pp_node *elif_grs, struct pp_node *node)
{
        _Bool status = false;
        if (pp_node_has_descendant(elif_grs)) {
                if (pp_node_elif_group_insert(
                        elif_grs->last_descendant, node))
                        return status = true;
        }
        if (node->type == pp_elif_group) {
                pp_node_add_descendant(elif_grs, node);
                status = true;
        }
        return status;
}


static inline _Bool
pp_node_elif_group_insert(struct pp_node *elif_gr, struct pp_node *node)
{
        return pp_node_if_group_insert(elif_gr, node);
}


static inline _Bool
pp_node_else_group_insert(struct pp_node *else_gr, struct pp_node *node)
{
        return pp_node_if_group_insert(else_gr, node);
}


static _Bool
pp_node_if_section_insert(struct pp_node *if_sect, struct pp_node *node)
{
        _Bool status = false;
        struct pp_node *n_node = NULL;
        if (pp_node_has_descendant(if_sect)) {
                struct pp_node *desc = if_sect->last_descendant;
                switch (desc->type) {
                case pp_if_group:
                {
                        if (pp_node_if_group_insert(desc, node))
                                return true;
                        switch (node->type) {
                        case pp_elif_group:
                        {
                                n_node = pp_node_create(pp_elif_groups);
                                if ((status = pp_node_elif_groups_insert(
                                        n_node, node)))
                                        pp_node_add_descendant(if_sect, n_node);
                                else
                                        pp_node_destroy(n_node);
                                return status;
                        }
                        case pp_else_group:
                        {
                                pp_node_add_descendant(if_sect, node);
                                return true;
                        }
                        default: 
                                break;
                        }
                        break;
                }
                case pp_elif_groups:
                {
                        if (pp_node_elif_groups_insert(desc, node))
                                return true;
                        if (node->type == pp_else_group) {
                                pp_node_add_descendant(if_sect, node);
                                return true;
                        }
                        break;
                }
                case pp_else_group:
                {
                        if (pp_node_else_group_insert(desc, node))
                                return true;
                        break;
                }
                case pp_endif_line:
                {
                        return false;
                }
                default:
                        break;
                }
                if (node->type == pp_endif_line) {
                        pp_node_add_descendant(if_sect, node);
                        return true;
                }
        } else if (node->type == pp_if_group) {
                pp_node_add_descendant(if_sect, node);
                return true;
        }
        return false;
}


static _Bool
pp_node_gr_part_insert(struct pp_node *gr_part, struct pp_node *node)
{
        _Bool status = false;
        if (pp_node_has_descendant(gr_part)) {
                struct pp_node *desc = gr_part->first_descendant;
                if (desc->type == pp_if_section)
                        status = pp_node_if_section_insert(desc, node);
                else
                        status = false;
        } else {
                switch (node->type) {
                case pp_if_group:
                {
                        struct pp_node *if_sect = pp_node_create(pp_if_section);
                        if ((status = pp_node_if_section_insert(if_sect, node)))
                                pp_node_add_descendant(gr_part, if_sect);
                        else
                                pp_node_destroy(if_sect);
                        break;
                }
                case pp_control_line:
                case pp_text_line:
                case pp_non_dir:
                        pp_node_add_descendant(gr_part, node);
                        status = true;
                        break;
                default:
                        break;
                }
        }
        return status;
}


static _Bool
pp_node_group_insert(struct pp_node *group, struct pp_node *node)
{
        _Bool status = false;
        if (pp_node_has_descendant(group)) {
                status = pp_node_gr_part_insert(group->last_descendant, node);
                if (status)
                        return true;
        }
        struct pp_node *gr_part = pp_node_create(pp_group_part);
        if ((status = pp_node_gr_part_insert(gr_part, node)))
                pp_node_add_descendant(group, gr_part);
        else
                pp_node_destroy(gr_part);
        return status;
}


_Bool
pp_node_file_insert(struct pp_node *file, struct pp_node *node)
{
        _Bool status = false;
        if (!pp_node_has_descendant(file)) {
                struct pp_node *group = pp_node_create(pp_group);
                if ((status = pp_node_group_insert(group, node)))
                        pp_node_add_descendant(file, group);
                else
                        pp_node_destroy(group);
        } else
                status = pp_node_group_insert(file->first_descendant, node);
        return status;
}