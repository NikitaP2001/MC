#include <pp.h>

const char* pp_get_log_fmt(enum MC_LOG_LEVEL loglevel)
{
        switch (loglevel) {
        case MC_FATAL:
                return "%s fatal: ";
        case MC_CRIT:
                return "%s critical: ";
        case MC_ERR:
                return "%s error: ";
        case MC_WARN:
                return "%s warning: ";
        case MC_DEBUG:
                return "%s trace: ";
        default:
                return "%s : ";
        }
}