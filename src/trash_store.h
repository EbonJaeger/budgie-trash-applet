#pragma once

#include "notify.h"
#include "trash_confirm_dialog.h"
#include "trash_info.h"
#include "trash_item.h"
#include "trash_settings.h"
#include "utils.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * The offset to the beginning of the line containing the
 * restore path.
 */
#define TRASH_INFO_PATH_OFFSET 13

#define TRASH_TYPE_STORE (trash_store_get_type())

G_DECLARE_FINAL_TYPE(TrashStore, trash_store, TRASH, STORE, GtkBox)

TrashStore *trash_store_new(gchar *drive_name, GIcon *icon, TrashSortMode mode);
TrashStore *trash_store_new_with_path(gchar *drive_name,
                                      TrashSortMode mode,
                                      GIcon *icon,
                                      gchar *trash_path);

void trash_store_start_monitor(TrashStore *self);

/**
 * Load all of the trashed items for this particular trashbin.
 * 
 * A new [TrashItem] widget is created for each item and added
 * to our list box.
 * 
 * If an error is encountered, `err` is set.
 */
gint trash_store_load_items(TrashStore *self, GError *err);

/**
 * Get the the number of trash items in this store.
 */
gint trash_store_get_count(TrashStore *self);

G_END_DECLS
