#ifndef _PP_NODE_H_
#define _PP_NODE_H_
#include <stdbool.h>

#include <list.h>
#include <pp/token.h>

enum pp_node_type {
        /* <group?> */
        pp_file,
        /* <gr_part>+ */
        pp_group,
        /* <if_section> | <control_line> | <#, non_dir> | <text_line> */
        pp_group_part,
        /* <if_group, elif_groups?, else_group?, endif_line> */
        pp_if_section,
        /* <#, if, const_expr,  new_line, group?> |
         * <#, ifdef, identifier, new_line, group?> |
         * <#, ifndef, identifier, new_line, group?> */
        pp_if_group,
        pp_const_expr,
        /* <elif_group>+ */
        pp_elif_groups,
        /* <#, elif, const_expr,  new_line, group?> */
        pp_elif_group,
        /* <#, else, new_line, group?> */
        pp_else_group,
        /* <#, endif, new_line> */
        pp_endif_line,
        pp_control_line,
        pp_non_dir,
        pp_text_line,
        pp_repl_list,
        pp_id_lst,
        pp_leaf,
        pp_tokens,
};

struct pp_node {
        struct list_head link;
        enum pp_node_type type;
        struct pp_token *leaf_tok;
        struct pp_node *first_descendant;
        struct pp_node *last_descendant;
};

struct pp_node*
pp_node_create(enum pp_node_type type);

/* all node resources are transited to @dest,
 * @dest is inserted into the list, instead @src 
 * @dest original list integrity is not preserved */ 
void pp_node_move(struct pp_node *dest, struct pp_node *src);

void 
pp_node_destroy(struct pp_node *node);

void pp_node_add_leaf(struct pp_node *node, struct pp_token *token);

void pp_node_add_descendant(struct pp_node *node, struct pp_node *ch_node);

struct pp_token *pp_node_leftmost_leaf(struct pp_node *node);

struct pp_token *pp_node_rightmost_leaf(struct pp_node *node);

_Bool
pp_node_file_insert(struct pp_node *file, struct pp_node *node);

#endif