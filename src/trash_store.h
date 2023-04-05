#pragma once

#include "notify.h"
#include "trash_info.h"
#include "utils.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define TRASH_TYPE_STORE (trash_store_get_type())

G_DECLARE_FINAL_TYPE(TrashStore, trash_store, TRASH, STORE, GObject)

TrashStore *trash_store_new_default(void);

TrashStore *trash_store_new_with_path(gchar *drive_name, GIcon *icon, gchar *trash_path);

GList *trash_store_get_items(TrashStore *self, GError *error);

gint trash_store_get_count(TrashStore *self);

GIcon *trash_store_get_icon(TrashStore *self);

const gchar *trash_store_get_name(TrashStore *self);

const gchar *trash_store_get_path(TrashStore *self);

gboolean trash_store_is_default(TrashStore *self);

G_END_DECLS
