#pragma once

#include "utils.h"
#include <gio/gio.h>

typedef struct {
    GObject parent_instance;

    gchar *file_name;
    const gchar *file_path;

    goffset size;

    gboolean is_directory;

    const gchar *restore_path;
    GDateTime *deleted_time;
} TrashInfo;

typedef struct {
    GObjectClass parent_class;
} TrashInfoClass;

/**
 * Create a new TrashInfo struct.
 */
TrashInfo *trash_info_new(gchar *file_name, const gchar *file_path, gboolean is_directory, goffset size);
