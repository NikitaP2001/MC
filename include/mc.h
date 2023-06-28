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

#define MC_ERR(message) printf("error: %s", message);

#define MC_WARN(message) printf("warning: %s", message);

#define TRACE(...)                      \
{                                               \
        int mlen = snprintf(NULL, 0, __VA_ARGS__); \
        char *message = malloc(mlen + 1); \
        snprintf(message, mlen + 1, __VA_ARGS__); \
        printf("[i] %s.%d: %s\n", __FILE__, __LINE__, message); \
        free(message); \
}

#endif /* _MC_H_ */