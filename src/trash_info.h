#pragma once

#include "utils.h"
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

typedef struct {
    GObject parent_instance;

    gchar *restore_path;
    GDateTime *deleted_time;
} TrashInfo;

typedef struct {
    GObjectClass parent_class;
} TrashInfoClass;

#define TRASH_TYPE_INFO (trash_info_get_type())
#define TRASH_INFO(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TRASH_TYPE_INFO, TrashInfo))
#define TRASH_IS_INFO(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRASH_TYPE_INFO))

/**
 * Create a new TrashInfo struct by reading a .trashinfo file.
 */
TrashInfo *trash_info_new_from_file(GFile *file);

/**
 * Create a new TrashInfo struct by reading a .trashinfo file
 * with a path prefix for the restore path.
 */
TrashInfo *trash_info_new_from_file_with_prefix(GFile *file, gchar *prefix);

G_END_DECLS
