#include <string.h>
#include <stdlib.h>

#include <tools.h>

void trie_init(struct trie_node *root, int invalid_id)
{
        memset(root, 0, sizeof(struct trie_node));
        root->word_id = invalid_id;
}

struct trie_node *trie_create(int id)
{
        struct trie_node *node = calloc(1, sizeof(struct trie_node));
        node->word_id = id;
        return node;
}

void trie_insert(struct trie_node *root, const char *restrict word, int id)
{
        int inv_id = root->word_id;
        struct trie_node *head = root;

        /* till the \0 end of @word */
        for (uint8_t curr; (curr = *word); word++) {
                if (head->child[curr] == NULL)
                        head->child[curr] = trie_create(inv_id);
                head = head->child[curr];
        }
        head->word_id = id;
}

int trie_search(struct trie_node *root, const char *restrict word, int length)
{
        struct trie_node *head = root;
        int id = root->word_id;

        for (int w_pos = 0; w_pos < length && head != NULL; w_pos++) {
                uint8_t curr = word[w_pos];
                head = head->child[curr];
        }
        if (head != NULL)
                id = head->word_id; 
        return id;
}

static void trie_node_free(struct trie_node *node)
{
        struct trie_node *child;
        for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
                child = node->child[i];
                if (child != NULL)
                        trie_node_free(child);
        }
        free(node);
}

void trie_free(struct trie_node *root)
{
        struct trie_node *child;
        for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
                child = root->child[i];
                if (child != NULL)
                        trie_node_free(child);
        }
}