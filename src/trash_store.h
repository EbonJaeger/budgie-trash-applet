#ifndef _BTA_TRASH_STORE_H
#define _BTA_TRASH_STORE_H

#include "trash_item.h"
#include "utils.h"
#include <gio/gio.h>
#include <glib.h>

/**
 * The offset to the beginning of the line containing the
 * restore path.
 */
#define TRASH_INFO_PATH_OFFSET 13

/**
 * The offset from the beginning of a line to the start of
 * the restore path in a trash info file.
 */
#define TRASH_INFO_PATH_PREFIX_OFFSET 5

/**
 * The offset to the beginning of the date from the start
 * of a line.
 */
#define TRASH_INFO_DELETION_DATE_PREFIX_OFFSET 14

/**
 * Struct representing a trash storage location.
 */
struct TrashStore;
typedef struct TrashStore {
    char *drive_name;
    const char *trashed_file_path;
    const char *trashed_info_path;
    GSList *trashed_items;
} TrashStore;

/**
 * Create a new TrashStore with the given file paths.
 * 
 * The TrashStore will be initialized with a NULL GSList
 * to hold trashed items, as per the GLib docs.
 * 
 * The return value of this should be freed with `trash_store_free()`.
 */
TrashStore *trash_store_new(char *drive_name, const char *trashed_file_path, const char *trashed_info_path);

/**
 * Free all resources used by a TrashStore struct.
 */
void trash_store_free(TrashStore *trash_store);

/**
 * Get a list of all files in the current user's trash bin.
 * 
 * If there was an error reading the trash directory, `NULL` will
 * be returned, and `err` will be set.
 */
void trash_load_items(TrashStore *trash_store, GError *err);

/**
 * Get a trashed item in the given trash bin. If there is no item
 * found with the given file name, NULL will be returned.
 */
TrashItem *trash_get_item_by_name(TrashStore *trash_store, const char *file_name);

gboolean trash_delete_item(TrashStore *trash_store,
                           TrashItem *trash_item,
                           GError *err);

/**
 * Restore an item from the trash bin to its original location.
 * 
 * If an error occurs, FALSE will be returned, and err will be set.
 */
gboolean trash_restore_item(TrashStore *trash_store,
                            TrashItem *trash_item,
                            GFileProgressCallback progress_callback,
                            gpointer progress_data,
                            GError **err);

#endif
