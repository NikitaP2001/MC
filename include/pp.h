#ifndef _PP_H_
#define _PP_H_

#include <stdarg.h>

#include <fs.h>
#include <pp/token.h>
#include <pp/lexer.h>
#include <pp/parser.h>

struct pp_context {
        struct filesys *fs;
        struct pp_node *root_file;
        struct pp_node *curr_group_part;

        enum mc_status last_err;
        size_t err_count;
};

enum mc_status pp_run(struct pp_context *pp, const char *start_file);

struct pp_token* pp_get_tu_start();

void pp_init(struct pp_context *pp, struct filesys *fs);

void pp_free(struct pp_context *pp);

static inline
void pp_set_status(struct pp_context *pp, enum mc_status status)
{
        pp->last_err = status;
}

static inline 
enum mc_status pp_get_status(struct pp_context *pp)
{
        return pp->last_err;
}

static inline
const char *pp_str_status(struct pp_context *pp)
{
        return mc_str_status(pp_get_status(pp));
}

void pp_print_token_line(struct pp_token *token);

void pp_print_node(struct pp_node *node);

void pp_error(struct pp_context *pp, const char *format, ...);


#endif /* _PP_H_ */