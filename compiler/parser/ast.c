#include <parser/ast.h>

static void pt_node_child_init(struct pt_child_nodes *child)
{
        child->size = 0;
        child->capacity = PT_CHILD_NODES_CAPACITY;
        child->nodes = (struct pt_node**)calloc(PT_CHILD_NODES_CAPACITY, 
                sizeof(struct pt_node*));
}

void pt_node_child_add(struct pt_node *node, struct pt_node *child)
{
        if (node->childs.size == node->childs.capacity) {
                node->childs.capacity *= 2;
                node->childs.nodes = (struct pt_node**)realloc(node->childs.nodes,
                        node->childs.capacity * sizeof(struct pt_node*));
        }
        child->parent = node;
        node->childs.nodes[node->childs.size++] = child;
}

static inline void pt_node_child_free(struct pt_child_nodes *child)
{
        free(child->nodes);
}

struct pt_node* pt_node_create(enum parser_symbol sym)
{
        struct pt_node *node = (struct pt_node *)
                calloc(1, sizeof(struct pt_node));
        pt_node_child_init(&node->childs);
        node->sym = sym;
        node->parent = NULL;
        return node;
}

struct pt_node* pt_node_create_leaf(struct token *tok)
{
        struct pt_node *node = pt_node_create(parser_token_tosymbol(tok));
        node->value = tok;
        return node;
}

void pt_node_destroy(struct pt_node *node)
{
        for (int i = 0; i < node->childs.size; i++)
                pt_node_destroy(node->childs.nodes[i]);
        pt_node_child_free(&node->childs);
        free(node);
}

enum parser_symbol pt_node_child_type(struct pt_node *node, uint32_t number)
{
        if (number >= node->childs.size)
                return psym_invalid;
        return node->childs.nodes[number]->sym;
}

struct token *ast_declarator_id(struct pt_node *decl)
{
        struct pt_node *dir_decl;

        while (decl->sym != psym_identifier) {
                dir_decl = pt_node_child_last(decl);
                decl = pt_node_child_first(dir_decl);
        }

        return decl->value;
}