#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fs.h>
#include <tools.h>

static const char basic_charset[] = "\t\n\v\f\r !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

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
                FILE *sfile = fopen(file->path, "rb");

                if (sfile != NULL) {
                        int fsize = get_file_size(sfile);
                        /* allocate for zero also and possibly newline */
                        file->content = calloc(fsize + 2, sizeof(char));
                        int read = fread(file->content, sizeof(char), fsize, sfile);

                        if (read != fsize) {
                                free(file->content);
                                file->content = NULL;
                                TRACE("read file %s failed", file->path);
                                return NULL;
                        }

                        if (file->content[read - 1] != '\n' && read != 0)
                                file->content[read++] = '\n';

                        file->content[read] = '\0';

                        for (int fp = 0; fp < read; fp++) {
                                char chr = file->content[fp];
                                if (!source_allowed_chr(chr)) {
                                        TRACE("invalid char 0x%02x", chr);
                                        file->last_error = MC_UNKNOWN_CHAR;
                                        free(file->content);
                                        file->content = NULL;
                                        break;
                                }
                        }

                        fclose(sfile);
                } else
                        TRACE("open file %s failed", file->path)
        }
        return file->content;
}


const char *source_name(struct fs_file *file)
{
        return fs_file_name(file->path);
}

static inline struct file_list *file_list_create()
{
        return calloc(1, sizeof(struct file_list));
}

static struct file_list *file_list_search(struct file_list *list, const char *f_name)
{
        LIST_FOREACH_ENTRY(list) {
                struct file_list *f_entry = (struct file_list*)entry;
                struct fs_file *e_file = &(f_entry)->file;
                const char *e_fname = source_name(e_file);
                if (strcmp(e_fname, f_name) == 0)
                        return f_entry;
        }
        return NULL;
}

static inline struct dir_list *dir_list_create()
{
        return calloc(1, sizeof(struct dir_list));
}

static struct file_list *dir_list_search(struct dir_list *list, const char *f_name)
{
        char f_path[_MAX_PATH];
        struct file_list *result = NULL;
        for (struct dir_list *dir = list; dir != NULL; dir = list_next(dir)) {
                strcpy(f_path, dir->entry.path);
                strcat(f_path, f_name);
                if (fs_isfile(f_path)) {
                        result = file_list_create();
                        strcpy(result->file.path, f_path);
                        break;
                }
        }
        return result;
}


static void dir_list_destroy(struct dir_list *head)
{
        do {
                struct dir_list *prev = head;
                head = list_next(head);
                free(list_prev(prev));
        } while (head != NULL);
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
        char *full_path = malloc(strlen(dir_path));
        strcpy(full_path, dir_path);
        struct dir_list *new_dir = dir_list_create();
        strcpy(new_dir->entry.path, full_path);
        if (fs->local_places == NULL)
                fs->local_places = new_dir;
        else
                list_insert(fs->local_places, new_dir);
        fs->last_error = MC_OK;
}


void fs_add_global(struct filesys *fs, 
const char *dir_path)
{
        char *full_path = malloc(strlen(dir_path));
        strcpy(full_path, dir_path);
        struct dir_list *new_dir = dir_list_create();
        strcpy(new_dir->entry.path, full_path);
        if (fs->global_places == NULL)
                fs->global_places = new_dir;
        else
                list_insert(fs->global_places, new_dir);
        fs->last_error = MC_OK;
}


struct fs_file *fs_get_local(struct filesys *fs, 
const char *f_name)
{
        struct file_list *result = NULL;

        result = file_list_search(fs->opened_local, f_name);
        if (result != NULL) 
                return &result->file;

        result = dir_list_search(fs->local_places, f_name);
        if (result != NULL) {
                if (fs->opened_local == NULL)
                        fs->opened_local = result;
                else
                        list_append(fs->opened_local, result);

                return &result->file;
        }
        
        return fs_get_global(fs, f_name);
}


struct fs_file *fs_get_global(struct filesys *fs, 
const char *f_name)
{
        struct file_list *result = NULL;
        result = file_list_search(fs->opened_global, f_name);
        if (result != NULL)
                return &result->file;

        result = dir_list_search(fs->global_places, f_name);
        if (result != NULL) {
                if (fs->opened_global == NULL)
                        fs->opened_global = result;
                else
                        list_append(fs->opened_global, result);
                return &result->file;
        }

        return NULL;
}

void fs_free(struct filesys *fs)
{
        if (fs->opened_global != NULL)
                list_destroy(fs->opened_global, free);
        if (fs->opened_local != NULL)
                list_destroy(fs->opened_local, free);
        if (fs->local_places != NULL)
                dir_list_destroy(fs->local_places);
        if (fs->global_places != NULL)
                dir_list_destroy(fs->global_places);
}