#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <source_provider.h>
#include <tools.h>

//static const char basic_charset[] = "a";

char_t *source_read(struct source_file *file)
{
        return (char*)file;
}


const char *source_name(struct source_file *file)
{
        return fs_file_name(file->path);
}


static struct file_list *file_list_search(struct file_list *list, const char *f_name)
{
        DLIST_FOREACH_ENTRY(list) {
                struct file_list *f_entry = (struct file_list*)entry;
                struct source_file *e_file = &(f_entry)->file;
                const char *e_fname = source_name(e_file);
                if (strcmp(e_fname, f_name) == 0)
                        return f_entry;
        }
        return NULL;
}

static struct file_list *dir_list_search(struct dir_list *list, const char *f_name)
{
        char f_path[_MAX_PATH];
        struct file_list *result = NULL;
        for (struct dir_list *dir = list; dir != NULL; dir = dlist_next(dir)) {
                strcpy(f_path, dir->entry.path);
                strcat(f_path, f_name);
                if (fs_isfile(f_path)) {
                        result = malloc(sizeof(struct file_list));
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
                head = dlist_next(head);
                free(dlist_prev(prev));
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


_Bool provider_add_local(struct source_provider *provider, 
const char *dir_path)
{
        char *full_path = fs_path(dir_path);
        if (full_path == NULL) {
                provider->last_error = MC_INVALID_PATH;
                return false;
        }
        struct dir_list *new_dir = malloc(sizeof(struct dir_list));
        strcpy(new_dir->entry.path, full_path);
        if (provider->local_places == NULL)
                provider->local_places = new_dir;
        else
                dlist_insert(provider->local_places, new_dir);
        provider->last_error = MC_OK;
        return true;
}


_Bool provider_add_global(struct source_provider *provider, 
const char *dir_path)
{
        char *full_path = fs_path(dir_path);
        if (full_path == NULL) {
                provider->last_error = MC_INVALID_PATH;
                return false;
        }
        struct dir_list *new_dir = malloc(sizeof(struct dir_list));
        strcpy(new_dir->entry.path, full_path);
        if (provider->global_places == NULL)
                provider->global_places = new_dir;
        else
                dlist_insert(provider->global_places, new_dir);
        provider->last_error = MC_OK;
        return true;
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
                dlist_insert(provider->opened_local, result);
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
                dlist_insert(provider->opened_global, result);
                return &result->file;
        }

        return NULL;
}

void provider_free(struct source_provider *provider)
{
        if (provider->opened_global != NULL)
                dlist_destroy(provider->opened_global, free);
        if (provider->opened_local != NULL)
                dlist_destroy(provider->opened_local, free);
        if (provider->local_places != NULL)
                dir_list_destroy(provider->local_places);
        if (provider->global_places != NULL)
                dir_list_destroy(provider->global_places);
}