#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <list.h>
#include <pp/parser.h>

static inline
struct pp_parser_pos 
pp_parser_get_pos(struct pp_parser *parser)
{
        return parser->pos;
}

static inline
void 
pp_parser_restore_pos(struct pp_parser *parser, struct pp_parser_pos pos)
{
        parser->pos = pos;
}

static inline
struct pp_token* 
pp_parser_advance(struct pp_parser *parser)
{
        parser->pos.tok = list_next(parser->pos.tok);
        return parser->pos.tok;
}

static inline
struct pp_token* 
pp_parser_token(struct pp_parser *parser)
{
        return parser->pos.tok;
}


static struct pp_node*
pp_parser_tokens(struct pp_parser *parser)
{
        struct pp_token *tok = pp_parser_token(parser);
        struct pp_node *toks = pp_node_create(pp_tokens);
        do {
                if (pp_token_valcmp(tok, "\n") == 0) {
                        if (toks->first_descendant == NULL)
                                goto discard;
                        break;
                }
                pp_node_add_leaf(toks, tok);
        } while ((tok = pp_parser_advance(parser)));
        return toks;
discard:
        pp_node_destroy(toks);
        return NULL;
}

static struct pp_node*
pp_parser_if_elif(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *if_gr = pp_node_create(pp_if_group);
        struct pp_token *tok = pp_parser_token(parser);

        if (pp_token_valcmp(tok, "#") != 0)
                goto discard;                         
        pp_node_add_leaf(if_gr, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "if") != 0) {
                if (pp_token_valcmp(tok, "elif") != 0)
                        goto discard;
                else
                        if_gr->type = pp_elif_group;
        }
        pp_node_add_leaf(if_gr, tok);
        tok = pp_parser_advance(parser);

        struct pp_node *c_expr = pp_node_create(pp_const_expr);
        pp_node_add_descendant(if_gr, c_expr);

        if (pp_token_valcmp(tok, "\n") == 0)
                goto discard;

        while (pp_token_valcmp(tok, "\n") != 0) {
                pp_node_add_leaf(c_expr, tok);
                tok = pp_parser_advance(parser);
        }
        pp_node_add_leaf(if_gr, tok);
        pp_parser_advance(parser);
        return if_gr;
discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(if_gr);
        return NULL;
}


static struct pp_node*
pp_parser_ifdef(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *if_gr = pp_node_create(pp_if_group);
        struct pp_token *tok = pp_parser_token(parser);

        if (pp_token_valcmp(tok, "#") != 0)
                goto discard;
        pp_node_add_leaf(if_gr, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "ifdef") != 0 && 
        pp_token_valcmp(tok, "ifndef") != 0)
                goto discard;
        pp_node_add_leaf(if_gr, tok);
        tok = pp_parser_advance(parser);

        if (tok->type != pp_id)
                goto discard;
        pp_node_add_leaf(if_gr, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "\n") != 0)
                goto discard;
        pp_node_add_leaf(if_gr, tok);
        pp_parser_advance(parser);
        return if_gr;
discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(if_gr);
        return NULL;
}


static struct pp_node*
pp_parser_else(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *else_gr = pp_node_create(pp_else_group);
        struct pp_token *tok = pp_parser_token(parser);

        if (pp_token_valcmp(tok, "#") != 0)
                goto discard;
        pp_node_add_leaf(else_gr, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "else") != 0)
                goto discard;
        pp_node_add_leaf(else_gr, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "\n") != 0)
                goto discard;
        pp_node_add_leaf(else_gr, tok);
        pp_parser_advance(parser);

        return else_gr;

discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(else_gr);
        return NULL;
}

static struct pp_node*
pp_parser_endif(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *endif_ln = pp_node_create(pp_endif_line);
        struct pp_token *tok = pp_parser_token(parser);

        if (pp_token_valcmp(tok, "#") != 0)
                goto discard;
        pp_node_add_leaf(endif_ln, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "endif") != 0)
                goto discard;
        pp_node_add_leaf(endif_ln, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "\n") != 0)
                goto discard;
        pp_node_add_leaf(endif_ln, tok);
        pp_parser_advance(parser);
        return endif_ln;

discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(endif_ln);
        return NULL;
}

static struct pp_node*
pp_parser_include(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *inc_ln = pp_node_create(pp_control_line);
        struct pp_token *tok = pp_parser_token(parser);
        struct pp_node *toks = NULL;

        if (pp_token_valcmp(tok, "#") != 0)
                goto discard;                         
        pp_node_add_leaf(inc_ln, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, PP_VAL_INCLUDE) != 0)
                goto discard;
        pp_node_add_leaf(inc_ln, tok);
        pp_parser_advance(parser);

        toks = pp_parser_tokens(parser);
        if (toks == NULL)
                goto discard;
        pp_node_add_descendant(inc_ln, toks);

        tok = pp_parser_token(parser);
        if (pp_token_valcmp(tok, "\n") != 0)
                goto discard;
        pp_node_add_leaf(inc_ln, tok);
        pp_parser_advance(parser);

        return inc_ln;
discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(inc_ln);
        return NULL;
}

static struct pp_node*
pp_parser_id_list(struct pp_parser *parser)
{
        struct pp_node *id_list = pp_node_create(pp_id_lst);
        struct pp_token *tok_id = pp_parser_token(parser);
        struct pp_token *tok_col = NULL;

        if (tok_id->type != pp_id)
                goto discard;
        pp_node_add_leaf(id_list, tok_id);
        
        tok_col = pp_parser_advance(parser);
        if (tok_col->type == pp_punct) {
                struct pp_parser_pos pos = pp_parser_get_pos(parser);
                tok_id = pp_parser_advance(parser); 
                while (tok_id->type == pp_id && tok_col->type == pp_punct
                && pp_token_valcmp(tok_col, ",") == 0) {
                        pp_node_add_leaf(id_list, tok_col);
                        pp_node_add_leaf(id_list, tok_id);
                        tok_col = pp_parser_advance(parser);
                        pos = pp_parser_get_pos(parser);
                        tok_id = pp_parser_advance(parser);       
                }
                pp_parser_restore_pos(parser, pos);
        }

        return id_list;
discard:
        pp_node_destroy(id_list);
        return NULL;
}

static struct pp_node*
pp_parser_repl(struct pp_parser *parser)
{
        struct pp_node *r_list = pp_node_create(pp_repl_list);
        struct pp_node *n_toks = pp_parser_tokens(parser);
        pp_node_add_descendant(r_list, n_toks);
        return r_list;
}

static struct pp_node*
pp_parser_define(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *def_ln = pp_node_create(pp_control_line);
        struct pp_node *desc = NULL;
        struct pp_token *tok = pp_parser_token(parser);

        if (pp_token_valcmp(tok, "#") != 0)
                goto discard;                         
        pp_node_add_leaf(def_ln, tok);

        tok = pp_parser_advance(parser);
        if (pp_token_valcmp(tok, "define") != 0)
                goto discard;
        pp_node_add_leaf(def_ln, tok);

        tok = pp_parser_advance(parser);
        if (tok->type != pp_id)
                goto discard;
        pp_node_add_leaf(def_ln, tok);

        tok = pp_parser_advance(parser);
        if (tok->type == pp_punct && pp_token_valcmp(tok, "(") == 0) {
                _Bool has_colon = false;
                tok = pp_parser_advance(parser);
                desc = pp_parser_id_list(parser);
                if (desc != 0) {
                        pp_node_add_descendant(def_ln, desc);
                        tok = pp_parser_token(parser);
                        if (tok->type == pp_punct 
                        && pp_token_valcmp(tok, ",") == 0) {
                                pp_node_add_leaf(def_ln, tok);
                                tok = pp_parser_advance(parser);
                                has_colon = true;
                        }
                }

                if (pp_token_valcmp(tok, ".") == 0) {
                        _Bool has_dots = false;
                        tok = pp_parser_advance(parser);
                        pp_node_add_leaf(def_ln, tok);
                        if (pp_token_valcmp(tok, ".") == 0) {
                                tok = pp_parser_advance(parser);
                                pp_node_add_leaf(def_ln, tok);
                                if (pp_token_valcmp(tok, ".") == 0) {
                                        tok = pp_parser_advance(parser);
                                        pp_node_add_leaf(def_ln, tok);
                                        has_dots = true;
                                }
                        }
                        if (!has_dots)
                                goto discard;

                } else if (has_colon)
                        goto discard;
                if (tok->type != pp_punct || pp_token_valcmp(tok, "(") != 0)
                        goto discard;
                pp_node_add_leaf(def_ln, tok);
                tok = pp_parser_advance(parser);
        }

        tok = pp_parser_advance(parser);
        if ((desc = pp_parser_repl(parser)) == NULL)
                goto discard;
        pp_node_add_descendant(def_ln, desc);
        pp_parser_advance(parser);

        if ((pp_token_valcmp(tok, "\n") != 0))
                goto discard;
        pp_parser_advance(parser);

discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(def_ln);
        return NULL;
}

static struct pp_node*
pp_parser_undef(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *undef = pp_node_create(pp_control_line);
        struct pp_token *tok = pp_parser_token(parser);

        if ((pp_token_valcmp(tok, "#") != 0))
                goto discard;                         
        pp_node_add_leaf(undef, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "undef") != 0)
                goto discard;                         
        pp_node_add_leaf(undef, tok);
        tok = pp_parser_advance(parser);

        if (tok->type != pp_id)
                goto discard;
        pp_node_add_leaf(undef, tok);
        tok = pp_parser_advance(parser);

        if ((pp_token_valcmp(tok, "\n") != 0))
                goto discard;
        pp_node_add_leaf(undef, tok);
        pp_parser_advance(parser);
        return undef;
discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(undef);
        return NULL;
}

static struct pp_node*
pp_parser_line(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *line = pp_node_create(pp_control_line);
        struct pp_node *toks = NULL;
        struct pp_token *tok = pp_parser_token(parser);

        if ((pp_token_valcmp(tok, "#") != 0))
                goto discard;                         
        pp_node_add_leaf(line, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "line") != 0)
                goto discard;                         
        pp_node_add_leaf(line, tok);
        pp_parser_advance(parser);

        toks = pp_parser_tokens(parser);
        if (toks == NULL)
                goto discard;
        pp_node_add_descendant(line, toks);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "\n") != 0)
                goto discard;
        pp_node_add_leaf(line, tok);
        pp_parser_advance(parser);
        return line;
discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(line);
        return NULL;
}

static struct pp_node*
pp_parser_error_pragma(struct pp_parser *parser)
{
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        struct pp_node *line = pp_node_create(pp_control_line);
        struct pp_node *toks = NULL;
        struct pp_token *tok = pp_parser_token(parser);

        if ((pp_token_valcmp(tok, "#") != 0))
                goto discard;                         
        pp_node_add_leaf(line, tok);
        tok = pp_parser_advance(parser);

        if (pp_token_valcmp(tok, "error") != 0 
                && pp_token_valcmp(tok, "pragma") != 0)
                goto discard;                         
        pp_node_add_leaf(line, tok);
        tok = pp_parser_advance(parser);

        toks = pp_parser_tokens(parser);
        if (toks != NULL) {
                pp_node_add_descendant(line, toks);
                tok = pp_parser_token(parser);
        }

        if ((pp_token_valcmp(tok, "\n") != 0))
                goto discard;
        pp_node_add_leaf(line, tok);
        pp_parser_advance(parser);
        return line;
discard:
        pp_parser_restore_pos(parser, pos);
        pp_node_destroy(line);
        return NULL;
}

static struct pp_node*
pp_parser_macroid(struct pp_parser *parser, struct pp_token *id)
{
        struct pp_node *macro = NULL;
        if (pp_token_valcmp(id, "if") == 0)
                macro = pp_parser_if_elif(parser);
        else if (pp_token_valcmp(id, "ifdef") == 0)
                macro = pp_parser_ifdef(parser);
        else if (pp_token_valcmp(id, "ifndef") == 0)
                macro = pp_parser_ifdef(parser);
        else if (pp_token_valcmp(id, "elif") == 0)
                macro = pp_parser_if_elif(parser);
        else if (pp_token_valcmp(id, "else") == 0)
                macro = pp_parser_else(parser);
        else if (pp_token_valcmp(id, "endif") == 0)
                macro = pp_parser_endif(parser);
        else if (pp_token_valcmp(id, PP_VAL_INCLUDE) == 0)
                macro = pp_parser_include(parser);
        else if (pp_token_valcmp(id, "define") == 0)
                macro = pp_parser_define(parser);
        else if (pp_token_valcmp(id, "undef") == 0)
                macro = pp_parser_undef(parser);
        else if (pp_token_valcmp(id, "line") == 0)
                macro = pp_parser_line(parser);
        else if (pp_token_valcmp(id, "error") == 0)
                macro = pp_parser_error_pragma(parser);
        else if (pp_token_valcmp(id, "pragma") == 0)
                macro = pp_parser_error_pragma(parser);
        return macro;
}

static struct pp_node*
pp_parser_macro(struct pp_parser *parser)
{
        struct pp_node *macro = NULL;
        struct pp_token *tok = pp_parser_token(parser);
        struct pp_parser_pos pos = pp_parser_get_pos(parser);
        if (tok == NULL)
                return macro;
        if (tok->type == pp_punct && pp_token_valcmp(tok, "#") == 0) {
                tok = pp_parser_advance(parser);
                if (tok->type == pp_id) {
                        pp_parser_restore_pos(parser, pos);
                        macro = pp_parser_macroid(parser, tok);
                } else if (pp_token_valcmp(tok, "\n") == 0) {
                        macro = pp_node_create(pp_control_line);
                        pp_node_add_leaf(macro, NULL);
                        pp_parser_advance(parser);
                }
        }
        if (macro == NULL)
                pp_parser_restore_pos(parser, pos);
        return macro;
}

static _Bool
pp_parser_line_toks(struct pp_parser *parser, struct pp_node *node)
{
        _Bool status = true;
        struct pp_node *toks = pp_parser_tokens(parser);
        if (toks != NULL)
                pp_node_add_descendant(node, toks);

        struct pp_token *tok = pp_parser_token(parser);
        if (pp_token_valcmp(tok, "\n") != 0) {
                status = false;
        } else {
                pp_node_add_leaf(node, tok);
                pp_parser_advance(parser);
        }
        return status;
}

static struct pp_node*
pp_parser_text_line(struct pp_parser *parser)
{
        struct pp_node *node = pp_node_create(pp_text_line);

        if (!pp_parser_line_toks(parser, node)) {
                pp_node_destroy(node);
                node = NULL;
        }
        return node;
}

static struct pp_node*
pp_parser_non_directive(struct pp_parser *parser)
{
        struct pp_node *node = pp_node_create(pp_non_dir);

        if (!pp_parser_line_toks(parser, node)) {
                pp_node_destroy(node);
                node = NULL;
        }
        return node;
}

void 
pp_parser_init(struct pp_parser *parser, struct pp_token *toks)
{
        parser->pos.tok = toks;
}

struct pp_node*
pp_parser_fetch_node(struct pp_parser *parser)
{
        struct pp_node *node = NULL;
        struct pp_token *tok = pp_parser_token(parser);

        node = pp_parser_macro(parser);
        if (node != NULL)
                return node;

        if (tok != NULL) {
                if (tok->type != pp_punct || pp_token_valcmp(tok, 
                        PP_VAL_HASH) != 0)
                        node = pp_parser_text_line(parser);
                else 
                        node = pp_parser_non_directive(parser);
        }

        return node;
}

