#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_REVEALER (trash_revealer_get_type())

G_DECLARE_FINAL_TYPE(TrashRevealer, trash_revealer, TRASH, REVEALER, GtkRevealer)

TrashRevealer *trash_revealer_new(void);
void trash_revealer_handle_clicked(GtkButton *sender, TrashRevealer *self);

void trash_revealer_set_text(TrashRevealer *self, gchar *text);

G_END_DECLS
