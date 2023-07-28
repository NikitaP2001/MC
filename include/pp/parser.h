#ifndef _PP_PARSER_H_
#define _pp_PARSER_H_

#include <pp/node.h>
#include <pp/token.h>

struct pp_parser_pos {
        struct pp_token *tok;
};

struct pp_parser {

        struct pp_parser_pos pos;

};

void 
pp_parser_init(struct pp_parser *parser, struct pp_token *toks);

struct pp_node*
pp_parser_fetch_node(struct pp_parser *parser);

#endif /* _PP_PARSER_H_ */