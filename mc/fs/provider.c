#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <source_provider.h>
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

enum mc_status source_lasterr(const struct source_file *file)
{
        return file->last_error; 
}

const char *source_read(struct source_file *file)
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


const char *source_name(struct source_file *file)
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
                struct source_file *e_file = &(f_entry)->file;
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


void provider_init(struct source_provider *provider)
{
        memset(provider, 0, sizeof(struct source_provider));
}


enum mc_status provider_lasterr(struct source_provider *provider)
{
        return provider->last_error;
}


void provider_add_local(struct source_provider *provider, 
const char *dir_path)
{
        char *full_path = malloc(strlen(dir_path));
        strcpy(full_path, dir_path);
        struct dir_list *new_dir = dir_list_create();
        strcpy(new_dir->entry.path, full_path);
        if (provider->local_places == NULL)
                provider->local_places = new_dir;
        else
                list_insert(provider->local_places, new_dir);
        provider->last_error = MC_OK;
}


void provider_add_global(struct source_provider *provider, 
const char *dir_path)
{
        char *full_path = malloc(strlen(dir_path));
        strcpy(full_path, dir_path);
        struct dir_list *new_dir = dir_list_create();
        strcpy(new_dir->entry.path, full_path);
        if (provider->global_places == NULL)
                provider->global_places = new_dir;
        else
                list_insert(provider->global_places, new_dir);
        provider->last_error = MC_OK;
}


struct source_file *provider_get_local(struct source_provider *provider, 
const char *f_name)
{
        struct file_list *result = NULL;

        result = file_list_search(provider->opened_local, f_name);
        if (result != NULL) 
                return &result->file;

        result = dir_list_search(provider->local_places, f_name);
        if (result != NULL) {
                if (provider->opened_local == NULL)
                        provider->opened_local = result;
                else
                        list_append(provider->opened_local, result);

                return &result->file;
        }
        
        return provider_get_global(provider, f_name);
}


struct source_file *provider_get_global(struct source_provider *provider, 
const char *f_name)
{
        struct file_list *result = NULL;
        result = file_list_search(provider->opened_global, f_name);
        if (result != NULL)
                return &result->file;

        result = dir_list_search(provider->global_places, f_name);
        if (result != NULL) {
                if (provider->opened_global == NULL)
                        provider->opened_global = result;
                else
                        list_append(provider->opened_global, result);
                return &result->file;
        }

        return NULL;
}

void provider_free(struct source_provider *provider)
{
        if (provider->opened_global != NULL)
                list_destroy(provider->opened_global, free);
        if (provider->opened_local != NULL)
                list_destroy(provider->opened_local, free);
        if (provider->local_places != NULL)
                dir_list_destroy(provider->local_places);
        if (provider->global_places != NULL)
                dir_list_destroy(provider->global_places);
}