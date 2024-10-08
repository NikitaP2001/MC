#ifndef _FS_TOOLS_H_
#define _FS_TOOLS_H_

#include <stdbool.h>
#include <stdio.h>

int fs_file_size(FILE *fp);

/* transform relative or abs. local system path to our internal path;
 * directory path should ends with '/' - when file path should not
 * Any other fs_* tools usage is recomended only after obtaining 
 * full path through that function
 *      return: null if not accesible, absolute path in other case
 *      note: returned path should be deallocated with free */
char *fs_path(const char *localpath);

_Bool fs_isfile(const char *path);

_Bool fs_isdir(const char *path);

_Bool fs_exist(const char *path);

const char *fs_path_name(const char *path);

#endif