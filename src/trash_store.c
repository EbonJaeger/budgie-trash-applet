#include "trash_store.h"
#include <stdio.h>

/**
 * Get the path to the trash info file for a given file
 * in the trash bin.
 * 
 * The result of this must be freed with `g_free()`.
 */
static const char *trash_get_info_file_path(const char *trash_info_path, const char *name)
{
    g_return_val_if_fail(trash_info_path != NULL, NULL);
    g_return_val_if_fail(name != NULL, NULL);

    // Build the trashinfo file name
    char *file_ext = ".trashinfo";
    gchar *info_file_name = (gchar *)malloc(1 + strlen(name) + strlen(file_ext));
    strcpy(info_file_name, name);
    strcat(info_file_name, file_ext);

    // Build the path to the trashinfo file
    const char *path = (const char *)g_build_path("/", trash_info_path, info_file_name, NULL);
    g_free(info_file_name);

    return path;
}

/**
 * Read the contents of a trashinfo file into memory.
 * 
 * The result of this should be freed with `free()`.
 */
static char *trash_read_trash_info(const char *trash_info_path, const char *file_name, GError **err)
{
    // Get the path to the trashinfo file
    const char *info_file_path = trash_get_info_file_path(trash_info_path, file_name);

    // Open the file
    GFile *info_file = g_file_new_for_path(info_file_path);
    GFileInputStream *input_stream = g_file_read(info_file, NULL, err);
    if (!input_stream)
    {
        g_object_unref(info_file);
        g_free((char *)info_file_path);
        return NULL;
    }

    // Seek to the Path line
    g_seekable_seek((GSeekable *)input_stream, TRASH_INFO_PATH_OFFSET, G_SEEK_SET, NULL, err);

    // Read the file contents and extract the line containing the restore path
    char *buffer = (char *)malloc(1024 * sizeof(char));
    gssize read;
    while ((read = g_input_stream_read((GInputStream *)input_stream, buffer, 1024, NULL, err)))
    {
        buffer[read] = '\0';
    }

    // Free some resources
    g_input_stream_close((GInputStream *)input_stream, NULL, NULL);
    g_object_unref(input_stream);
    g_object_unref(info_file);
    g_free((char *)info_file_path);

    return buffer;
}

/**
 * Parse the contents of a trashinfo file to get the time
 * that a file was deleted.
 * 
 * The result of this should be unreffed with `g_date_time_unref()`.
 */
static GDateTime *trash_get_deletion_date(char *data)
{
    int substr_start = (int)(strchr(data, '\n') - data + TRASH_INFO_DELETION_DATE_PREFIX_OFFSET);
    int length = strlen(data) - substr_start - 1;

    char *deletion_date_str = (char *)malloc(length + 1);

    deletion_date_str = substring(data, deletion_date_str, substr_start, length);
    deletion_date_str[length] = '\0';

    GTimeZone *tz = g_time_zone_new_local();
    GDateTime *deletion_date = g_date_time_new_from_iso8601((const gchar *)deletion_date_str, tz);
    g_time_zone_unref(tz);
    free(deletion_date_str);

    return deletion_date;
}

/**
 * Parse the contents of a trashinfo file to get the path
 * that a trashed file would be restored to.
 * 
 * The result of this should be freed with `free()`.
 */
static char *trash_get_restore_path(char *data)
{
    int end_of_line = (int)(strchr(data, '\n') - data);
    int length = end_of_line - TRASH_INFO_PATH_PREFIX_OFFSET;

    char *restore_path = (char *)malloc(length + 1);

    restore_path = substring(data, restore_path, TRASH_INFO_PATH_PREFIX_OFFSET, length);
    restore_path[length] = '\0';

    return restore_path;
}

TrashStore *trash_store_new(char *drive_name, const char *trashed_file_path, const char *trashed_info_path)
{
    g_return_val_if_fail(drive_name != NULL, NULL);
    g_return_val_if_fail(trashed_file_path != NULL, NULL);
    g_return_val_if_fail(trashed_info_path != NULL, NULL);

    TrashStore *trash_store = (TrashStore *)malloc(sizeof(TrashStore));
    if (!trash_store)
    {
        return NULL;
    }

    trash_store->drive_name = drive_name;
    trash_store->trashed_file_path = trashed_file_path;
    trash_store->trashed_info_path = trashed_info_path;
    trash_store->trashed_items = NULL;

    return trash_store;
}

void trash_store_free(TrashStore *trash_store)
{
    g_return_if_fail(trash_store != NULL);

    free(trash_store->drive_name);
    free((char *)trash_store->trashed_file_path);
    free((char *)trash_store->trashed_info_path);
    if (trash_store->trashed_items)
    {
        g_slist_free_full(trash_store->trashed_items, (GDestroyNotify)trash_item_free);
    }
    free(trash_store);
}

void trash_load_items(TrashStore *trash_store, GError *err)
{
    g_return_if_fail(trash_store != NULL);
    g_return_if_fail(trash_store->trashed_items == NULL);

    // Open our trash directory
    GFile *trash_dir = g_file_new_for_path(trash_store->trashed_file_path);
    GFileEnumerator *enumerator = g_file_enumerate_children(trash_dir,
                                                            G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            NULL,
                                                            &err);
    if G_UNLIKELY (!enumerator)
    {
        // There was a problem getting the enumerator; return early
        g_object_unref(trash_dir);
        return;
    }

    // Iterate over the directory's children and append each file name to a list
    GFileInfo *current_file;
    while ((current_file = g_file_enumerator_next_file(enumerator, NULL, &err)))
    {
        const char *file_name = g_file_info_get_name(current_file);
        char *trashed_path = (char *)g_build_path("/", trash_store->trashed_file_path, file_name, NULL);

        // Parse the trashinfo file for this item
        char *trash_info_contents = trash_read_trash_info(trash_store->trashed_info_path, file_name, &err);
        if G_UNLIKELY (!trash_info_contents)
        {
            break;
        }

        char *restore_path = trash_get_restore_path(trash_info_contents);
        GDateTime *deletion_date = trash_get_deletion_date(trash_info_contents);
        TrashInfo *trash_info = trash_info_new(restore_path, deletion_date);

        TrashItem *trash_item = trash_item_new_with_info(strdup(file_name), trashed_path, trash_info);
        g_object_unref(current_file);
        g_warn_if_fail(trash_item != NULL);

        free(trash_info_contents);
        trash_store->trashed_items = g_slist_append(trash_store->trashed_items, (gpointer)trash_item);
    }

    // Free resources
    g_file_enumerator_close(enumerator, NULL, NULL);
    g_object_unref(enumerator);
    g_object_unref(trash_dir);
}

TrashItem *trash_get_item_by_name(TrashStore *trash_store, const char *file_name)
{
    g_return_val_if_fail(trash_store != NULL, NULL);
    g_return_val_if_fail(file_name != NULL, NULL);

    TrashItem *trash_item;
    guint length = g_slist_length(trash_store->trashed_items);
    for (int i = 0; i < length; i++)
    {
        trash_item = g_slist_nth_data(trash_store->trashed_items, i);
        if (strcmp(trash_item->name, file_name) == 0)
        {
            break;
        }
        trash_item = NULL;
    }

    return trash_item;
}

gboolean trash_delete_item(TrashStore *trash_store,
                           TrashItem *trash_item,
                           GError *err)
{
    g_return_val_if_fail(trash_store != NULL, FALSE);
    g_return_val_if_fail(trash_item != NULL, FALSE);

    // Delete the trashed file (if it's a directory, it will delete recursively)
    gboolean success = delete_trashed_file(trash_item->path, trash_item->is_directory, err);
    if (!success)
    {
        return success;
    }

    // Delete the .trashinfo file
    const char *info_file_path = trash_get_info_file_path(trash_store->trashed_info_path, trash_item->name);
    GFile *info_file = g_file_new_for_path(info_file_path);
    success = g_file_delete(info_file, NULL, &err);

    g_object_unref(info_file);
    g_free((char *)info_file_path);

    return success;
}

gboolean trash_restore_item(TrashStore *trash_store,
                            TrashItem *trash_item,
                            GFileProgressCallback progress_callback,
                            gpointer progress_data,
                            GError **err)
{
    GFile *trashed_file = g_file_new_for_path(trash_item->path);
    GFile *restored_file = g_file_new_for_path((const char *)trash_item->trash_info->restore_path);

    gboolean success = g_file_move(trashed_file,
                                   restored_file,
                                   G_FILE_COPY_ALL_METADATA,
                                   NULL,
                                   progress_callback,
                                   progress_data,
                                   err);

    if (!success)
    {
        g_object_unref(trashed_file);
        g_object_unref(restored_file);
        return success;
    }

    // Delete the .trashinfo file
    const char *info_file_path = trash_get_info_file_path(trash_store->trashed_info_path, trash_item->name);
    GFile *info_file = g_file_new_for_path(info_file_path);
    success = g_file_delete(info_file, NULL, err);

    g_object_unref(trashed_file);
    g_object_unref(restored_file);
    g_object_unref(info_file);
    g_free((char *)info_file_path);

    return success;
}
