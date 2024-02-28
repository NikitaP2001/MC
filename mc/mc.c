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
                return "[fatal] %s.%d: ";
        case MC_CRIT:
                return "[crit] %s.%d: ";
        case MC_ERR:
                return "[err] %s.%d: ";
        case MC_WARN:
                return "[warn] %s.%d: ";
        case MC_DEBUG:
                return "[trace] %s.%d: ";
        default:
                return "%s.%d: ";
        }
}

static _Bool mc_is_init = false;

void mc_init()
{
        if (!mc_is_init) {
                token_init();
                mc_is_init = true;
        }
        
}

_Bool mc_isinit()
{
        return mc_is_init;
}