#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fs.h>

static const char basic_charset[] = "\t\n\v\f\r !\"#$%&'()*+,-./0123456789:;<=>?@"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

_Bool source_allowed_chr(char chr)
{
        _Bool result = false;
        for (size_t sp = 0; sp < sizeof(basic_charset) - 1; sp++) {
                if (basic_charset[sp] == chr)
                        result = true;
        }

        return result;
}

enum mc_status source_lasterr(const struct fs_file *file)
{
        return file->last_error; 
}

const char *source_read(struct fs_file *file)
{
        if (file->content == NULL) {
                FILE *sfile = fopen(file->path, "r");

                if (sfile != NULL) {
                        int fsize = fs_file_size(sfile);
                        /* allocate for zero also and possibly newline */
                        file->content = calloc(fsize + 2, sizeof(char));
                        int read = fread(file->content, sizeof(char), fsize, sfile);

                        if (read != fsize && !feof(sfile)) {
                                free(file->content);
                                file->content = NULL;
                                MC_LOG(MC_DEBUG, "read file %s failed", file->path);
                                return NULL;
                        }

                        if (file->content[read - 1] != '\n' && read != 0)
                                file->content[read++] = '\n';

                        file->content[read] = '\0';

                        for (int fp = 0; fp < read; fp++) {
                                char chr = file->content[fp];
                                if (!source_allowed_chr(chr)) {
                                        MC_LOG(MC_DEBUG, "invalid char 0x%02x", chr);
                                        file->last_error = MC_UNKNOWN_CHAR;
                                        free(file->content);
                                        file->content = NULL;
                                        break;
                                }
                        }

                        fclose(sfile);
                } else
                        MC_LOG(MC_DEBUG, "open file %s failed", file->path);
        }
        return file->content;
}


const char *source_name(struct fs_file *file)
{
        return fs_file_name(file->path);
}

static inline struct fs_file *fs_file_create(const char *f_path)
{
        struct fs_file *file = calloc(1, sizeof(struct fs_file));
        file->path = malloc((strlen(f_path) + 1) * sizeof(char));
        strcpy(file->path, f_path);
        return file;
}

static struct fs_file *fs_file_search(struct fs_file *list, const char *f_name)
{
        LIST_FOREACH_ENTRY(list) {
                struct fs_file *f_entry = (struct fs_file*)entry;
                const char *e_fname = source_name(f_entry);
                if (strcmp(e_fname, f_name) == 0)
                        return f_entry;
        }
        return NULL;
}

static void fs_file_destroy(struct fs_file *list)
{
        if (list->content != NULL)
                free(list->content);
        free(list->path);
        free(list);
}

static inline struct fs_dir *fs_dir_create(const char *d_path)
{
        struct fs_dir *dir = calloc(1, sizeof(struct fs_dir));
        dir->path = malloc((strlen(d_path) + 1) * sizeof(char));
        strcpy(dir->path, d_path);
        return dir;
}

static struct fs_file *fs_dir_file_search(struct fs_dir *list, const char *f_name)
{
        struct fs_file *result = NULL;
        size_t f_path_len = 0;
        size_t path_len = 0;
        char *f_path = NULL;

        for (struct fs_dir *dir = list; dir != NULL; dir = list_next(dir)) {

                path_len = strlen(dir->path) + strlen(f_name);
                if (f_path_len < path_len || f_path == NULL) {
                        f_path = malloc((path_len + 1) * sizeof(char));
                        f_path_len = path_len;
                }

                strcpy(f_path, dir->path);
                strcat(f_path, f_name);

                if (fs_isfile(f_path)) {
                        result = fs_file_create(f_path);
                        break;
                }
        }
        if (f_path != NULL)
                free(f_path);

        return result;
}


static void fs_dir_destroy(struct fs_dir *dir)
{
        free(dir->path);
        free(dir);
}


void fs_init(struct filesys *fs)
{
        memset(fs, 0, sizeof(struct filesys));
}


enum mc_status fs_lasterr(struct filesys *fs)
{
        return fs->last_error;
}


void fs_add_local(struct filesys *fs, 
const char *dir_path)
{
        struct fs_dir *new_dir = fs_dir_create(dir_path);
        if (fs->local_places == NULL)
                fs->local_places = new_dir;
        else
                list_append(fs->local_places, new_dir);
        fs->last_error = MC_OK;
}


void fs_add_global(struct filesys *fs, 
const char *dir_path)
{
        struct fs_dir *new_dir = fs_dir_create(dir_path);
        if (fs->global_places == NULL)
                fs->global_places = new_dir;
        else
                list_insert(fs->global_places, new_dir);
        fs->last_error = MC_OK;
}


struct fs_file *fs_get_local(struct filesys *fs, 
const char *f_name)
{
        struct fs_file *result = NULL;

        result = fs_file_search(fs->opened_local, f_name);
        if (result != NULL) 
                return result;

        result = fs_dir_file_search(fs->local_places, f_name);
        if (result != NULL) {
                if (fs->opened_local == NULL)
                        fs->opened_local = result;
                else
                        list_append(fs->opened_local, result);

                return result;
        }
        
        return fs_get_global(fs, f_name);
}


struct fs_file *fs_get_global(struct filesys *fs, 
const char *f_name)
{
        struct fs_file *result = NULL;
        result = fs_file_search(fs->opened_global, f_name);
        if (result != NULL)
                return result;

        result = fs_dir_file_search(fs->global_places, f_name);
        if (result != NULL) {
                if (fs->opened_global == NULL)
                        fs->opened_global = result;
                else
                        list_append(fs->opened_global, result);
                return result;
        }

        return NULL;
}

void fs_free(struct filesys *fs)
{
        if (fs->opened_global != NULL)
                list_destroy(fs->opened_global, (list_free)fs_file_destroy);
        if (fs->opened_local != NULL)
                list_destroy(fs->opened_local, (list_free)fs_file_destroy);
        if (fs->local_places != NULL)
                list_destroy(fs->local_places, (list_free)fs_dir_destroy);
        if (fs->global_places != NULL)
                list_destroy(fs->global_places, (list_free)fs_dir_destroy);
}