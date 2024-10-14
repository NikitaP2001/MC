#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <mc.h>
#include <token.h>

const char *mc_status_table[] = {
        "no error",
        "epic fail",
        "file operation failed",
        "memory allocation failed",
        "invalid argument provided",
        "failed to open a file",
        "the filesystem path is invalid",
        "unknown character found",
        "syntax error",
        "file not found",
        "parse error"
};

const char* mc_get_log_fmt(enum MC_LOG_LEVEL loglevel)
{
        switch (loglevel) {
        case MC_FATAL:
                return "[fatal] %s:%d: ";
        case MC_CRIT:
                return "[crit] %s:%d: ";
        case MC_ERR:
                return "[err] %s:%d: ";
        case MC_WARN:
                return "[warn] %s:%d: ";
        case MC_DEBUG:
                return "[trace] %s:%d: ";
        default:
                return "%s:%d: ";
        }
}

static struct {

        _Bool mc_is_init;

        /* mc print related messages log level is applied 
        * depending on last logged mc_msg level */
        enum MC_LOG_LEVEL last_level;

        enum MC_LOG_LEVEL level;

} mc_global;

static inline _Bool mc_log_isopen()
{
        return (mc_global.last_level <= mc_global.level);
}

/* Relate to last logged event using mc_msg */
int mc_printf(const char *format, ...)
{
        int n_print = 0;
        va_list args;
        assert(mc_isinit());

        if (mc_log_isopen()) {
                va_start(args, format);
                n_print = vprintf(format, args);
                va_end(args);
        }
        return n_print;
}

/* Relate to last logged event using mc_msg */
int mc_putchar(int c)
{
        int c_put = EOF;
        assert(mc_isinit());
        if (mc_log_isopen())
                c_put = putchar(c);
        return c_put;
}

int mc_vprintf(const char *format, va_list args)
{
        int n_print = 0;
        assert(mc_isinit());
        if (mc_log_isopen())
                n_print = vprintf(format, args);
        return n_print;
}

int mc_msg(enum MC_LOG_LEVEL level, const char *format, ...)
{
        int n_print = 0;
        va_list args;
        mc_global.last_level = level;
        if (mc_log_isopen()) {
                va_start(args, format);
                n_print = vprintf(format, args);
                va_end(args);
        }
        return n_print;
}

void mc_init_args(int argc, char *argv[])
{
        for (int iarg = 0; iarg < argc; iarg++) {
                if (strcmp(argv[iarg], "-M") == 0 && iarg + 1 < argc) {
                        long level = strtol(argv[++iarg], NULL, 0);
                        if (level <= MC_LVL_MAX && level > 0)
                                mc_global.level = level;
                        else 
                                mc_msg(MC_ERR, "failed to parse arg -M");
                }
        }
}

void mc_init(int argc, char *argv[])
{
        if (!mc_isinit()) {
                token_global_init();
                mc_global.mc_is_init = true;
                mc_global.last_level = MC_LVL_MAX;
                mc_global.level = MC_FATAL;
                if (argc != 0 && argv != NULL)
                        mc_init_args(argc, argv);
        }
}

void mc_free()
{
        if (mc_isinit()) {
                token_global_free();
                mc_global.mc_is_init = false;
        }
}

_Bool mc_isinit()
{
        return mc_global.mc_is_init;
}

#define TRAP_ABORT() __asm__ volatile ("int3;")

#ifdef DEBUG

int __mc_log(int loglevel, const char *file, size_t line,
              const char *format, ...)
{
        va_list args;
        if (loglevel <= MC_CRIT) TRAP_ABORT();
        if (loglevel <= (int)mc_global.level) {
                printf(mc_get_log_fmt(loglevel), file, line);
                va_start(args, format);
                vprintf(format, args);
                putchar('\n');
                va_end(args);
        }
        
        return 0;
}

#endif