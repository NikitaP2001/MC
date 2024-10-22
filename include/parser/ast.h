#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_
#include <stdint.h>
#include <stdlib.h>

#include <parser.h>

#define PT_CHILD_NODES_CAPACITY 5

/* Be aware that ast differs from how grammar defines 
 * parse tree stucture, so consider examining ast building procedures
 * in parse.c, before working with the tree.
 * The main points are: 
 *  - all excessive characters are omitted, when possible
 *    example: (type-name) in postfix expr will go without ()
 *  - recurrent symbol placement avoided, when possible
 *    example: trainslation-unit: translation-unit external-declaration
 *    is a rule defined by grammar, when in ast all external-declaration 
 *    will be childs of a single translation unit, to avoid recursion 
 */

struct pt_child_nodes {
        struct pt_node **nodes; /* may be pt_node_leaf also */
        uint16_t size;
        uint16_t capacity;
};

struct pt_node {
        struct pt_node *parent;
        enum parser_symbol sym;
        struct pt_child_nodes childs;  
        struct token *value;
};

#define AST_FOREACH_CHILD(node)                                                 \
        for (uint16_t i = 0,  *entry = (uint16_t *)node->childs.nodes[i];       \
                i < node->childs.size;                                          \
                i++, entry = (uint16_t *)node->childs.nodes[i])


static inline uint16_t pt_node_child_count(struct pt_node *node)
{
        return node->childs.size;
}

static inline _Bool pt_node_child_empty(struct pt_node *node)
{
        return (node->childs.size == 0);
}

struct pt_node* pt_node_create(enum parser_symbol sym);

struct pt_node* pt_node_create_leaf(struct token *tok);

void pt_node_child_add(struct pt_node *node, struct pt_node *child);

void pt_node_destroy(struct pt_node *node);

enum parser_symbol pt_node_child_type(struct pt_node *node, uint32_t number);

static inline struct pt_node *pt_node_child_remove(struct pt_node *node)
{
        assert(node->childs.size != 0);
        return node->childs.nodes[--node->childs.size];
}

static inline struct pt_node *pt_node_child_last(struct pt_node *node)
{
        if (node->childs.size == 0)
                return NULL;
        return node->childs.nodes[node->childs.size - 1];
}

static inline struct pt_node *pt_node_child_first(struct pt_node *node)
{
        if (node->childs.size == 0)
                return NULL;
        return node->childs.nodes[0];
}

static inline 
struct pt_node *pt_node_child_number(struct pt_node *node, uint16_t num)
{
        if (node->childs.size > num || num == 0)
                return NULL;
        return node->childs.nodes[num - 1];
}

struct token *ast_declarator_id(struct pt_node *decl);

#endif /* _PARSE_TREE_H_ */