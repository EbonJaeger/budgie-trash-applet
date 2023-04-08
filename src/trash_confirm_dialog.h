#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_CONFIRM_DIALOG (trash_confirm_dialog_get_type())

G_DECLARE_FINAL_TYPE(TrashConfirmDialog, trash_confirm_dialog, TRASH, CONFIRM_DIALOG, GtkRevealer)

TrashConfirmDialog *trash_confirm_dialog_new(const gchar *message, gboolean destructive);

gboolean trash_confirm_dialog_get_destructive(TrashConfirmDialog *self);

const gchar *trash_confirm_dialog_get_message(TrashConfirmDialog *self);

void trash_confirm_dialog_set_destructive(TrashConfirmDialog *self, gboolean destructive);

void trash_confirm_dialog_set_message(TrashConfirmDialog *self, const gchar *message);

G_END_DECLS
