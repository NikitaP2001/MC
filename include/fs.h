#ifndef _SOURCE_PROVIDER_H_
#define _SOURCE_PROVIDER_H_
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
        char *content;
        char path[_MAX_PATH];

        enum mc_status last_error;
};

enum mc_status source_lasterr(const struct fs_file *file);

const char *source_read(struct fs_file *file);

const char *source_name(struct fs_file *file);

struct dir_list {
        struct list_head link;
        struct dir_entry {
                char path[_MAX_PATH];
        } entry;
};

struct file_list {
        struct list_head link;
        struct fs_file file;
};

struct filesys {

        struct dir_list *local_places, *global_places;

        struct file_list *opened_local, *opened_global;

        enum mc_status last_error;
};

void fs_init(struct filesys *provider);

enum mc_status fs_lasterr(struct filesys *provider);

/* returns false if path is not valid */
void fs_add_local(struct filesys *provider, 
const char *dir_path);

void fs_add_global(struct filesys *provider, 
const char *dir_path);

/* If multiple files with the same name without path specified could be 
 * found in global or local places - we will return first matching. */

/* if failed to find in local, will the apply search in global places */
struct fs_file *fs_get_local(struct filesys *prov, 
const char *f_name);

/* On the contrary, unlike get_local will search only in global places */
struct fs_file *fs_get_global(struct filesys *prov, 
const char *f_name);

void fs_release_file(struct filesys *provider, 
struct fs_file *file);

void fs_free(struct filesys *provider);

#endif /* _SOURCE_PROVIDER_H_ */