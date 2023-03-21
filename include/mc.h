#ifndef _MC_H_
#define _MC_H_

enum mc_status {
        MC_OK = 0,
        MC_FILE_OP_FAIL = 1,
        MC_MEMALLOC_FAIL = 2,
        MC_OPEN_FILE_FAIL = 3,
};

extern const char *mc_status_table[];

#define MC_SUCC(status) (status == 0)

static inline const char *mc_str_status(int status)
{
        return mc_status_table[status];
}

#endif /* _MC_H_ */
