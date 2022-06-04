#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_CONFIRM_DIALOG (trash_confirm_dialog_get_type ())

G_DECLARE_FINAL_TYPE (TrashConfirmDialog, trash_confirm_dialog, TRASH, CONFIRM_DIALOG, GtkRevealer)

/**
 * Create a new TrashConfirmDialog widget.
 */
TrashConfirmDialog *trash_confirm_dialog_new (void);

/**
 * Set the message for the revealer to show.
 * 
 * If `destructive` is set to true, the confirm button will have
 * the destructive CSS class applied to it.
 */
void trash_confirm_dialog_show_message (TrashConfirmDialog *self, const gchar *message, gboolean destructive);

G_END_DECLS
