#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_STORE (trash_store_get_type())

G_DECLARE_FINAL_TYPE(TrashStore, trash_store, TRASH, STORE, GtkBox)

TrashStore *trash_store_new(gchar *drive_name, gchar *icon_name);
void trash_store_set_drive_name(TrashStore *self, gchar *drive_name);
void trash_store_set_icon_name(TrashStore *self, gchar *icon_name);

G_END_DECLS
