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
void trash_store_set_btns_sensitive(TrashStore *self, gboolean sensitive);
void trash_store_check_empty(TrashStore *self);

/*
 * Property functions
 */

void trash_store_set_drive_name(TrashStore *self, gchar *drive_name);
void trash_store_set_icon_name(TrashStore *self, gchar *icon_name);
void trash_store_set_trash_path(TrashStore *self, gchar *trash_path);
void trash_store_set_trashinfo_path(TrashStore *self, gchar *trashinfo_path);

/*
 * UI signal handlers
 */

void trash_store_handle_header_btn_clicked(GtkButton *sender, TrashStore *self);
void trash_store_handle_cancel_clicked(TrashRevealer *sender, TrashStore *self);
void trash_store_handle_confirm_clicked(TrashRevealer *sender, TrashStore *self);
void trash_store_handle_row_activated(GtkListBox *sender, GtkListBoxRow *row, TrashStore *self);

/*
 * Trash handling functions
 */

/**
 * Handles file events for this store's trash directory.
 * 
 * We handle G_FILE_MONITOR_EVENT_MOVED_IN, G_FILE_MONITOR_EVENT_MOVED_OUT, and
 * G_FILE_MONITOR_EVENT_DELETED events, adding and removing TrashItems
 * as needed.
 */
void trash_store_handle_monitor_event(GFileMonitor *monitor,
                                      GFile *file,
                                      GFile *other_file,
                                      GFileMonitorEvent event_type,
                                      TrashStore *self);

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
 * Create a new [TrashItem] from a GFileInfo.
 * 
 * If an error is encountered, `err` is set and `NULL` is returned.
 * 
 * TODO: Maybe this should be in TrashItem instead.
 */
TrashItem *trash_store_create_trash_item(TrashStore *self, GFileInfo *file_info, GError **err);

/**
 * Read the contents of a trashinfo file for a file with the given name
 * into memory.
 * 
 * If an error is encountered, `err` is set and `NULL` is returned.
 */
gchar *trash_store_read_trash_info(gchar *trashinfo_path, GError **err);

/**
 * Sorts the trash items in the file box widget by the following rules:
 * 
 * 1. Directories should be above regular files
 * 2. Directories should be sorted alphabetically
 * 3. Files should be sorted alphabetically
 */
gint trash_store_sort_by_type(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer user_data);

/**
 * Compare the names of a [TrashItem] to see if it matches.
 */
gint trash_store_compare_items(TrashItem *a, gchar *name);

G_END_DECLS
