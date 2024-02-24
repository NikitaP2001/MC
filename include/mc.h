#ifndef _MC_H_
#define _MC_H_
#include <stdio.h>

enum mc_status {
        MC_OK = 0,
        MC_FAIL = 1,
        MC_FILE_OP_FAIL = 2,
        MC_MEMALLOC_FAIL = 3,
        MC_INVALID_ARG = 4,
        MC_OPEN_FILE_FAIL = 5,
        MC_INVALID_PATH = 6,
        MC_UNKNOWN_CHAR = 7,
        MC_SYNTAX_ERR   = 8,
        MC_FILE_NOT_FOUND = 9,
        MC_PARSE_ERROR = 10,
};

extern const char *mc_status_table[];

static inline _Bool __mc_succ(enum mc_status status)
{
        return (status == 0);
}
#define MC_SUCC(status) __mc_succ(status)

static inline const char *mc_str_status(int status)
{
        return mc_status_table[status];
}

typedef size_t file_size_t;

enum MC_LOG_LEVEL {
        MC_FATAL = 0,
        MC_CRIT = 1,
        MC_ERR = 2,
        MC_WARN = 3,
        MC_DEBUG = 4,
};

#ifdef DEBUG

const char* mc_get_log_fmt(enum MC_LOG_LEVEL loglevel);

#define MC_LOG(loglevel, ...)                                           \
{                                                                       \
        printf(mc_get_log_fmt(loglevel), __FILE__, __LINE__);           \
        printf(__VA_ARGS__);                                            \
        putchar('\n');                                                  \
}

#else /* DEBUG */

#define MC_LOG(...) do {} while (0)

#endif /* DEBUG */

#define UNUSED(var) (void)(var)

#endif /* _MC_H_ */