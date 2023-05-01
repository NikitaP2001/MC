#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/stat.h>

#include <tools.h>
#include <mc.h>

int get_file_size(FILE *fp)
{
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        return size;
}

#ifdef _WIN32

static inline char *realpath(const char *restrict path,
                      char *restrict resolved_path)
{
        return _fullpath(resolved_path, path, _MAX_PATH);
}

#endif /* _WIN32 */

char *fs_path(const char *localpath)
{
        char *fullp = realpath(localpath, NULL);
        size_t plen = strlen(fullp);
        if (plen > _MAX_PATH) {
                MC_WARN("Path exceeded the maximum limit");
                free(fullp);
                return NULL;
        }
#ifdef _WIN32
        for (size_t i = 0; i < plen; i++) {
                if (fullp[i] == '\\')
                        fullp[i] = '/';
        }
#endif
        return fullp;
}


_Bool fs_isfile(const char *path)
{
        struct stat fst = {0};
        return stat(path, &fst) != -1 && S_ISREG(fst.st_mode);
}

_Bool fs_isdir(const char *path)
{
        struct stat dst = {0};
        return stat(path, &dst) != -1 && S_ISDIR(dst.st_mode);
}

_Bool fs_exist(const char *path)
{
        struct stat st = {0};
        return stat(path, &st) != -1;
}


const char *fs_file_name(const char *path)
{
        const char *n_pos = NULL;
        if (fs_isfile(path)) {
                for ( ;*path != '\0'; path++) {
                        if (*path == '/')
                                n_pos = path;
                }
        }
        return n_pos;
}