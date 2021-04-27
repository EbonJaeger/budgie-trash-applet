#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

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
#define TRASH_INFO_DELETION_DATE_PREFIX_OFFSET 13

#define TRASH_TYPE_INFO (trash_info_get_type())

G_DECLARE_FINAL_TYPE(TrashInfo, trash_info, TRASH, INFO, GObject);

/**
 * Create a new TrashInfo struct by reading a .trashinfo file.
 */
TrashInfo *trash_info_new_from_file(GFile *file);

/**
 * Create a new TrashInfo struct by reading a .trashinfo file
 * with a path prefix for the restore path.
 */
TrashInfo *trash_info_new_from_file_with_prefix(GFile *file, gchar *prefix);

/**
 * Get the restore path for this TrashInfo struct.
 *
 * This returns a clone of the path.
 */
gchar *trash_info_get_restore_path(TrashInfo *self);

/**
 * Get the deletion date for this TrashInfo struct.
 * 
 * This function increases the ref count of the
 * returned GDateTime.
 */
GDateTime *trash_info_get_deletion_time(TrashInfo *self);

/**
 * Formats the deletion date for use in GTK widgets.
 */
gchar *trash_info_format_deletion_time(TrashInfo *self);

void trash_info_set_restore_path(TrashInfo *self, gchar *path);
void trash_info_set_deletion_time(TrashInfo *self, GDateTime *timestamp);

G_END_DECLS
