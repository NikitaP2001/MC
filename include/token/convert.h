#ifndef _CONVERT_H_
#define _CONVERT_H_
#include <mc.h>

struct convert_context {
        struct pp_token *pos;
        struct token* first;
        struct token* last;
        enum mc_status status;
};

static inline struct pp_token*
convert_pos(struct convert_context *ctx)
{
        return ctx->pos;
}

void convert_setpos(struct convert_context *ctx, struct pp_token *tok);

static inline _Bool 
convert_continue(struct convert_context *ctx)
{
        return MC_SUCC(ctx->status);
}

_Bool convert_advance(struct convert_context *ctx);

void convert_retreat(struct convert_context *ctx);

void convert_init(struct convert_context *ctx, struct pp_context *pp);

void convert_error(struct convert_context *ctx, const char *format, ...);

/* this implements convertation described in C99 standart 
 * with the following steps:
 *
 * Each source character set member and escape sequence 
 * in character constants and string literals is converted 
 * to the corresponding member of the execution character 
 * set; if there is no corresponding member, it is converted
 * to an implementation-defined member other than the null 
 * (wide) character.
 *
 * Adjacent string literal tokens are concatenated. 
 * 
 * White-space characters separating tokens are no longer significant. Each 
 * preprocessing token is converted into a token. */
enum mc_status convert_run(struct convert_context *ctx);

struct token *convert_get_token(struct convert_context *ctx);

void convert_free(struct convert_context *ctx);

#endif /* _CONVERT_H_ */