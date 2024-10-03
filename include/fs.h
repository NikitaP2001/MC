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
struct fs_file {
        struct list_head link;
        struct filesys *fs;
        size_t use_count;
        char *content;
        char *path;
};

struct filesys {

        struct fs_dir *local_places, *global_places;

        struct fs_file *opened_local, *opened_global;

        enum mc_status last_error;
};

static inline
enum mc_status fs_file_lasterr(const struct fs_file *file)
{
        return file->fs->last_error;
}

static inline
void fs_file_seterr(struct fs_file *file, enum mc_status status)
{
        file->fs->last_error = status;
}

const char *fs_file_read(struct fs_file *file);

const char *fs_file_name(struct fs_file *file);

static inline
const char *
fs_file_path(struct fs_file *file)
{
        return file->path;
}

struct fs_dir {
        struct list_head link;
        struct filesys *fs;
        char *path;
};

void fs_init(struct filesys *fs);

static inline
enum mc_status fs_lasterr(struct filesys *fs)
{
        return fs->last_error;
}

static inline
enum mc_status fs_seterr(struct filesys *fs, enum mc_status status)
{
        return fs->last_error = status;
}

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

static inline 
struct fs_file*
fs_share_file(struct fs_file *file)
{
        file->use_count += 1;
        return file;
}

void fs_release_file(struct fs_file *file);

void fs_free(struct filesys *fs);

#endif /* _SOURCE_PROVIDER_H_ */