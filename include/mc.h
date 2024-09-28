#ifndef _MC_H_
#define _MC_H_
#include <stdio.h>
#include <stdint.h>

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

typedef enum mc_status mc_status_t;

#define _IN
#define _OUT
#define _INOUT

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

typedef long unsigned int file_size_t;

enum MC_LOG_LEVEL {
        MC_FATAL = 0,
        MC_CRIT = 1,
        MC_ERR = 2,
        MC_WARN = 3,
        MC_DEBUG = 4,
};

#ifdef DEBUG

const char* mc_get_log_fmt(enum MC_LOG_LEVEL loglevel);

int __mc_log(int loglevel, const char *file, size_t line,
              const char *format, ...);

#define MC_LOG(loglevel, ...)                                   \
        __mc_log(loglevel, __FILE__, __LINE__, __VA_ARGS__)

#else /* DEBUG */

#define MC_LOG(...) do {} while (0)

#endif /* DEBUG */

#define UNUSED(var) (void)(var)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/* one time initialization of the compiler module */
void mc_init();

void mc_free();

_Bool mc_isinit();

/* integer types related to target environment */
typedef long long unsigned int mc_u64_t;
typedef unsigned int           mc_u32_t;
typedef unsigned short int     mc_u16_t;
typedef unsigned char          mc_u8_t;
typedef mc_u16_t               mc_wchar_t;
typedef mc_u8_t                mc_char_t;

#define MC_U64_MAX            ULLONG_MAX
#define MC_U64_MIN            ULLONG_MIN
#define MC_U32_MAX            UINT_MAX
#define MC_U32_MIN            UINT_MIN
#define MC_U16_MAX            USHRT_MAX
#define MC_U16_MIN            USHRT_MIN
#define MC_U8_MAX             UCHAR_MAX
#define MC_U8_MIN             UCHAR_MIN

#define MC_WCHAR_MAX          MC_U16_MAX
#define MC_WCHAR_MIN          MC_U16_MIN
#define MC_UCHAR_MAX          MC_U8_MAX
#define MC_UCHAR_MIN          MC_U8_MIN

/* is character an octal digit */
static inline _Bool isodigit(int c)
{
        return (isdigit(c) && c < 8);
}

#endif /* _MC_H_ */