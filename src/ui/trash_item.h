#pragma once

#include "trash_revealer.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_ITEM (trash_item_get_type())

G_DECLARE_FINAL_TYPE(TrashItem, trash_item, TRASH, ITEM, GtkBox)

TrashItem *trash_item_new(gchar *name,
                          gchar *path,
                          gchar *trashinfo_path,
                          gchar *restore_path,
                          GIcon *icon,
                          gboolean is_directory,
                          gchar *timestamp);
void trash_item_apply_button_styles(TrashItem *self);
void trash_item_set_btns_sensitive(TrashItem *self, gboolean sensitive);

void trash_item_set_icon(TrashItem *self, GIcon *icon);
void trash_item_set_file_name(TrashItem *self, gchar *file_name);
void trash_item_set_path(TrashItem *self, gchar *path);
void trash_item_set_trashinfo_path(TrashItem *self, gchar *path);
void trash_item_set_restore_path(TrashItem *self, gchar *path);
void trash_item_set_directory(TrashItem *self, gboolean is_directory);
void trash_item_set_timestamp(TrashItem *self, gchar *timestamp);

void trash_item_handle_btn_clicked(GtkButton *sender, TrashItem *self);
void trash_item_handle_cancel_clicked(TrashRevealer *sender, TrashItem *self);
void trash_item_handle_confirm_clicked(TrashRevealer *sender, TrashItem *self);

void trash_item_toggle_info_revealer(TrashItem *self);

gboolean trash_item_delete(TrashItem *self, GError **err);
gboolean trash_item_restore(TrashItem *self, GError **err);

G_END_DECLS
