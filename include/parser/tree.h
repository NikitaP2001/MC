#ifndef _PARSE_TREE_H_
#define _PARSE_TREE_H_
#include <stdint.h>
#include <stdlib.h>

#include <parser.h>

#define PT_CHILD_NODES_CAPACITY 5

struct pt_child_nodes {
        struct pt_node **nodes; /* may be pt_node_leaf also */
        uint16_t size;
        uint16_t capacity;
};

struct pt_node {
        enum parser_symbol sym;
        struct pt_child_nodes childs;  
        struct token *value;
};

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
        assert(node->childs.size != 0);
        return node->childs.nodes[node->childs.size - 1];
}

#endif /* _PARSE_TREE_H_ */