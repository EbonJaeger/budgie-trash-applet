#pragma once

#include "utils.h"
#include <gio/gio.h>

G_BEGIN_DECLS

/**
 * All of the file attributes that we need to query for to build a
 * TrashInfo struct.
 */
#define TRASH_FILE_ATTRIBUTES G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_ICON "," G_FILE_ATTRIBUTE_STANDARD_SIZE "," G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_TRASH_DELETION_DATE "," G_FILE_ATTRIBUTE_TRASH_ORIG_PATH

#define TRASH_TYPE_INFO (trash_info_get_type ())

G_DECLARE_FINAL_TYPE (TrashInfo, trash_info, TRASH, INFO, GObject)

struct _TrashInfoClass {
    GObjectClass parent_class;
};

/**
 * Create a new TrashInfo struct.
 *
 * If there is an error getting the file info for the file
 * at the URI, `NULL` will be returned and `err` will be set.
 */
TrashInfo *trash_info_new (const gchar *uri, GError *err);

/* Property getters */

const gchar *trash_info_get_name (TrashInfo *self);

const gchar *trash_info_get_uri (TrashInfo *self);

const gchar *trash_info_get_restore_path (TrashInfo *self);

GIcon *trash_info_get_icon (TrashInfo *self);

goffset trash_info_get_size (TrashInfo *self);

gboolean trash_info_is_directory (TrashInfo *self);

GDateTime *trash_info_get_deletion_time (TrashInfo *self);

/* Property setters */

void trash_info_set_file_name (TrashInfo *self, const gchar *value);

void trash_info_set_file_uri (TrashInfo *self, const gchar *value);

void trash_info_set_restore_path (TrashInfo *self, const gchar *value);

void trash_info_set_icon (TrashInfo *self, GIcon *icon);

void trash_info_set_file_size (TrashInfo *self, goffset value);

void trash_info_set_is_directory (TrashInfo *self, gboolean value);

void trash_info_set_deletion_time (TrashInfo *self, GDateTime *value);

G_END_DECLS
