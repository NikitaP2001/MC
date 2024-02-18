#ifndef _PP_H_
#define _PP_H_

#include <fs.h>
#include <pp/token.h>
#include <pp/lexer.h>
#include <pp/parser.h>

struct pp_context {
        struct filesys *fs;
        struct pp_node *root_file;
};

const char* pp_get_log_fmt(enum MC_LOG_LEVEL loglevel);

#define PP_MSG(msg_lvl, ...)                                            \
{                                                                       \
        printf(mc_get_log_fmt(msg_lvl));                                \
        printf(__VA_ARGS__);                                            \
        putchar('\n');                                                  \
}


#endif /* _PP_H_ */