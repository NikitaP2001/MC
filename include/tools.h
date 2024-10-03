#ifndef _TOOLS_H_
#define _TOOLS_H_
#include <stdint.h>
#include <limits.h>

#include <tools/hashtable.h>

static inline int64_t max(int64_t num1, int64_t num2)
{
        return (num1 > num2 ? num1 : num2);
}

#define TRIE_ALPHABET_SIZE UCHAR_MAX

struct trie_node {
        struct trie_node *child[TRIE_ALPHABET_SIZE];
        /* in the root, invalid id is stored, as well as in
         * the non-end nodes */
        int word_id; 
};

void trie_init(struct trie_node *root, int invalid_id);

void trie_insert(struct trie_node *root, const char *restrict word, int id);

/* returns word id associated with given input*/
int trie_search(struct trie_node *root, const char *restrict word, int length);

void trie_free(struct trie_node *root);

#endif /* _TOOLS_H_ */