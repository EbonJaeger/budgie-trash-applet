#pragma once

#include "notify.h"
#include "trash_info.h"
#include "trash_item.h"
#include "trash_revealer.h"
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
#define TRASH_STORE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TRASH_TYPE_STORE, TrashStore))
#define TRASH_IS_STORE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRASH_TYPE_STORE))

typedef struct _TrashStore TrashStore;
typedef struct _TrashStoreClass TrashStoreClass;

TrashStore *trash_store_new(gchar *drive_name, GIcon *icon, TrashSortMode mode);
TrashStore *trash_store_new_with_path(gchar *drive_name,
                                      TrashSortMode mode,
                                      GIcon *icon,
                                      gchar *trash_path);
void trash_store_apply_button_styles(TrashStore *self);
void trash_store_set_btns_sensitive(TrashStore *self, gboolean sensitive);
void trash_store_check_empty(TrashStore *self);
void trash_store_start_monitor(TrashStore *self);

/*
 * UI signal handlers
 */

gboolean trash_store_handle_header_clicked(GtkWidget *sender, GdkEventButton *event, TrashStore *self);
void trash_store_handle_header_btn_clicked(GtkButton *sender, TrashStore *self);
void trash_store_handle_cancel_clicked(GtkButton *sender, TrashStore *self);
void trash_store_handle_confirm_clicked(GtkButton *sender, TrashStore *self);
void trash_store_handle_row_activated(GtkListBox *sender, GtkListBoxRow *row, TrashStore *self);

/*
 * Trash handling functions
 */

/**
 * Constructs a URI from a `GFileInfo` and the `TrashStore` file location.
 * 
 * If the store is the default store, the URI will have the format `trash:///example.txt`.
 * Otherwise, in the case of mounts, the function will escape the path to the file because
 * that is what seems to happen when GLib(?) puts items in the `trash` URI scheme.
 */
GUri *trash_store_get_uri_for_file(TrashStore *self, const gchar *file_name);

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
gint trash_store_load_items(TrashStore *self, GError *err);

/**
 * Sorts the trash items in the file box widget.
 */
gint trash_store_sort(GtkListBoxRow *row1, GtkListBoxRow *row2, TrashStore *self);

gint trash_store_get_count(TrashStore *self);

GType trash_store_get_type(void) G_GNUC_CONST;

G_END_DECLS
