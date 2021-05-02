#pragma once

#include "utils.h"
#include <gio/gio.h>

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

/**
 * Create a new TrashInfo struct by reading a .trashinfo file.
 */
TrashInfo *trash_info_new_from_file(GFile *file);

/**
 * Create a new TrashInfo struct by reading a .trashinfo file
 * with a path prefix for the restore path.
 */
TrashInfo *trash_info_new_from_file_with_prefix(GFile *file, gchar *prefix);
