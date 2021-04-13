#pragma once

#include "trash_item.h"
#include "trash_revealer.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * The offset to the beginning of the line containing the
 * restore path.
 */
#define TRASH_INFO_PATH_OFFSET 13

#define TRASH_TYPE_STORE (trash_store_get_type())

G_DECLARE_FINAL_TYPE(TrashStore, trash_store, TRASH, STORE, GtkBox)

TrashStore *trash_store_new(gchar *drive_name, gchar *icon_name);
TrashStore *trash_store_new_with_paths(gchar *drive_name, gchar *icon_name, gchar *trash_path, gchar *trashinfo_path);
void trash_store_apply_button_styles(TrashStore *self);

void trash_store_set_drive_name(TrashStore *self, gchar *drive_name);
void trash_store_set_icon_name(TrashStore *self, gchar *icon_name);
void trash_store_set_trash_path(TrashStore *self, gchar *trash_path);
void trash_store_set_trashinfo_path(TrashStore *self, gchar *trashinfo_path);

void trash_store_set_btns_sensitive(TrashStore *self, gboolean sensitive);
void trash_store_check_empty(TrashStore *self);

void trash_store_handle_header_btn_clicked(GtkButton *sender, TrashStore *self);
void trash_store_handle_cancel_clicked(TrashRevealer *sender, TrashStore *self);
void trash_store_handle_confirm_clicked(TrashRevealer *sender, TrashStore *self);

/**
 * Load all of the trashed items for this particular trashbin.
 * 
 * A new [TrashItem] widget is created for each item and added
 * to our list box.
 * 
 * If an error is encountered, `err` is set.
 */
void trash_store_load_items(TrashStore *self, GError *err);

/**
 * Read the contents of a trashinfo file for a file with the given name
 * into memory.
 * 
 * If an error is encountered, `err` is set and `NULL` is returned.
 */
gchar *trash_store_read_trash_info(TrashStore *self, const gchar *file_name, GError *err);

/**
 * Sorts the trash items in the file box widget by the following rules:
 * 
 * 1. Directories should be above regular files
 * 2. Directories should be sorted alphabetically
 * 3. Files should be sorted alphabetically
 */
gint trash_store_sort_by_type(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer user_data);

G_END_DECLS
