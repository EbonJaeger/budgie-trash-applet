#pragma once

#include "trash_revealer.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_STORE (trash_store_get_type())

G_DECLARE_FINAL_TYPE(TrashStore, trash_store, TRASH, STORE, GtkBox)

TrashStore *trash_store_new(gchar *drive_name, gchar *icon_name);
void trash_store_apply_button_styles(TrashStore *self);
void trash_store_set_drive_name(TrashStore *self, gchar *drive_name);
void trash_store_set_icon_name(TrashStore *self, gchar *icon_name);
void trash_store_set_btns_sensitive(TrashStore *self, gboolean sensitive);

void trash_store_handle_header_btn_clicked(GtkButton *sender, TrashStore *self);
void trash_store_handle_cancel_clicked(TrashRevealer *sender, TrashStore *self);
void trash_store_handle_confirm_clicked(TrashRevealer *sender, TrashStore *self);

G_END_DECLS