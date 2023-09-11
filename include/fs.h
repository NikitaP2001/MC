#ifndef _SOURCE_PROVIDER_H_
#define _SOURCE_PROVIDER_H_
#include <fs/tools.h>
#include <list.h>
#include <mc.h>

/* physical source file multibyte characters are mapped into 
 * implementation defined manner, to the source character set
 * (including new-line characters for end-of-line indicators)
 * - Trigraph sequences are replcaed by corresponding 
 * single-character internal representations.
 * A source file that is not empty shall end in a new-line character.
 * note: current path limit is _MAX_PATH
 */

/* note: source file and other fs places should be identified
 * by full path, which can be obtained using tools::fs_path */
struct fs_file {
        struct list_head link;
        char *content;
        char *path;

        enum mc_status last_error;
};

enum mc_status source_lasterr(const struct fs_file *file);

const char *source_read(struct fs_file *file);

const char *source_name(struct fs_file *file);

struct fs_dir {
        struct list_head link;
        char *path;
};

struct filesys {

        struct fs_dir *local_places, *global_places;

        struct fs_file *opened_local, *opened_global;

        enum mc_status last_error;
};

void fs_init(struct filesys *fs);

enum mc_status fs_lasterr(struct filesys *fs);

/* returns false if path is not valid */
void fs_add_local(struct filesys *fs, 
const char *dir_path);

void fs_add_global(struct filesys *fs, 
const char *dir_path);

/* If multiple files with the same name without path specified could be 
 * found in global or local places - we will return first matching. */

/* if failed to find in local, will the apply search in global places */
struct fs_file *fs_get_local(struct filesys *fs, 
const char *f_name);

/* On the contrary, unlike get_local will search only in global places */
struct fs_file *fs_get_global(struct filesys *fs, 
const char *f_name);

void fs_release_file(struct filesys *fs, 
struct fs_file *file);

void fs_free(struct filesys *fs);

#endif /* _SOURCE_PROVIDER_H_ */