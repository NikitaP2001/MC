#include <pp.h>

#define PP_MODULE_NAME "preprocessor"

const char* pp_get_log_fmt(enum MC_LOG_LEVEL loglevel)
{
        switch (loglevel) {
        case MC_FATAL:
                return PP_MODULE_NAME " fatal: ";
        case MC_CRIT:
                return PP_MODULE_NAME " critical: ";
        case MC_ERR:
                return PP_MODULE_NAME " error: ";
        case MC_WARN:
                return PP_MODULE_NAME " warning: ";
        case MC_DEBUG:
                return PP_MODULE_NAME " trace: ";
        default:
                return PP_MODULE_NAME " : ";
        }
}