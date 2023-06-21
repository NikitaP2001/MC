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
struct source_file {
        char *content;
        char path[_MAX_PATH];

        enum mc_status last_error;
};

enum mc_status source_lasterr(const struct source_file *file);

const char *source_read(struct source_file *file);

const char *source_name(struct source_file *file);

struct dir_list {
        struct list_head link;
        struct dir_entry {
                char path[_MAX_PATH];
        } entry;
};

struct file_list {
        struct list_head link;
        struct source_file file;
};

struct source_provider {

        struct dir_list *local_places, *global_places;

        struct file_list *opened_local, *opened_global;

        enum mc_status last_error;
};

void provider_init(struct source_provider *provider);

enum mc_status provider_lasterr(struct source_provider *provider);

/* returns false if path is not valid */
void provider_add_local(struct source_provider *provider, 
const char *dir_path);

void provider_add_global(struct source_provider *provider, 
const char *dir_path);

/* If multiple files with the same name without path specified could be 
 * found in global or local places - we will return first matching. */

/* if failed to find in local, will the apply search in global places */
struct source_file *provider_get_local(struct source_provider *prov, 
const char *f_name);

/* On the contrary, unlike get_local will search only in global places */
struct source_file *provider_get_global(struct source_provider *prov, 
const char *f_name);

void provider_release_file(struct source_provider *provider, 
struct source_file *file);

void provider_free(struct source_provider *provider);

#endif /* _SOURCE_PROVIDER_H_ */