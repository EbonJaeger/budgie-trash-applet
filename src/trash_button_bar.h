#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_BUTTON_BAR (trash_button_bar_get_type())

G_DECLARE_FINAL_TYPE(TrashButtonBar, trash_button_bar, TRASH, BUTTON_BAR, GtkBox)

TrashButtonBar *trash_button_bar_new(void);

GtkWidget *trash_button_bar_add_button(TrashButtonBar *self, const gchar *text, gint response_id);

GtkWidget *trash_button_bar_get_content_area(TrashButtonBar *self);

gboolean trash_button_bar_get_revealed(TrashButtonBar *self);

void trash_button_bar_add_response_style_class(TrashButtonBar *self, gint response_id, const gchar *style);

void trash_button_bar_set_response_sensitive(TrashButtonBar *self, gint response_id, gboolean sensitive);

void trash_button_bar_set_revealed(TrashButtonBar *self, gboolean reveal);

G_END_DECLS
