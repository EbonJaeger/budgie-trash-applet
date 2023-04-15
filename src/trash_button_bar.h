#pragma once

#include <gtk/gtk.h>
#include <stdlib.h>

G_BEGIN_DECLS

#define TRASH_TYPE_BUTTON_BAR (trash_button_bar_get_type())

G_DECLARE_FINAL_TYPE(TrashButtonBar, trash_button_bar, TRASH, BUTTON_BAR, GtkBox)

TrashButtonBar *trash_button_bar_new(void);

GtkWidget *trash_button_bar_add_button(TrashButtonBar *self, const gchar *text, gint response_id);

void trash_button_bar_set_response_sensitive(TrashButtonBar *self, gint response_id, gboolean sensitive);

G_END_DECLS
