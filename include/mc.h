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
};

extern const char *mc_status_table[];

#define MC_SUCC(status) (status == 0)

static inline const char *mc_str_status(int status)
{
        return mc_status_table[status];
}

enum MC_LOG_LEVEL {
        MC_LOGFATAL = 0,
        MC_LOGCRIT = 1,
        MC_LOGERR = 2,
        MC_LOGWARN = 3,
        MC_LOGDEBUG = 4,
};

#ifdef DEBUG

static inline const char* mc_get_log_fmt(enum MC_LOG_LEVEL loglevel)
{
        switch (loglevel) {
        case MC_LOGFATAL:
                return "[fatal] %s.%d: %s\n";
        case MC_LOGCRIT:
                return "[crit] %s.%d: %s\n";
        case MC_LOGERR:
                return "[err] %s.%d: %s\n";
        case MC_LOGWARN:
                return "[warn] %s.%d: %s\n";
        case MC_LOGDEBUG:
                return "[trace] %s.%d: %s\n";
        default:
                return "%s.%d: %s\n";
        }
}

#define MC_LOG(loglevel, ...)                                           \
{                                                                       \
        int mlen = snprintf(NULL, 0, __VA_ARGS__);                      \
        char *message = malloc(mlen + 1);                               \
        snprintf(message, mlen + 1, __VA_ARGS__);                       \
        printf(mc_get_log_fmt(loglevel), __FILE__, __LINE__, message);  \
        free(message);                                                  \
}

#else /* DEBUG */

#define MC_LOG(...) do {} while (0)

#endif /* DEBUG */

#endif /* _MC_H_ */