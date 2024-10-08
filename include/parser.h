#ifndef _PARSER_H_
#define _PARSER_H_
#include <stdbool.h>

#include <list.h>
#include <parser/symbol.h>
#include <parser/ast.h>
#include <parser/symtable.h>
#include <mc.h>
#include <stack.h>

#define PARSER_STACK_CAPACITY 25

struct parser_ops {
        struct token*           (*pull_token)(void *data);
        void                    (*put_token)(void *data, struct token *tok);
        struct token*           (*fetch_token)(void *data);
        mc_status_t             (*error)(void *data, const char *message);
};

struct parser {
        /* is used to store top level unfinished production */
        struct stack stack;
        /* for fetched lookahed production */
        struct pt_node *lookahead;
        struct parser_ops ops;
        void *data; /* some user provided data, is passed to ops */
        struct hash_table id_tbl;
};

struct pt_node *parser_translation_unit(struct parser *ps);

enum parser_symbol parser_token_tosymbol(struct token *tok);

void parser_init(struct parser *ps, struct parser_ops ops, void *user_data);

void parser_free(struct parser *ps);

#endif /* _PARSER_H_ */