#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum {
    TRASH_CONFIRM_RESPONSE_OK = 0,
    TRASH_CONFIRM_RESPONSE_CANCEL = 1
} TrashConfirmResponseType;

#define TRASH_TYPE_CONFIRM_DIALOG (trash_confirm_dialog_get_type ())
#define TRASH_CONFIRM_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TRASH_TYPE_CONFIRM_DIALOG, TrashConfirmDialog))
#define TRASH_CONFIRM_DIALOG_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST((obj), TRASH_TYPE_CONFIRM_DIALOG, TrashConfirmDialogClass))
#define TRASH_IS_CONFIRM_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRASH_TYPE_CONFIRM_DIALOG))
#define TRASH_IS_CONFIRM_DIALOG_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((obj), TRASH_TYPE_CONFIRM_DIALOG))
#define TRASH_CONFIRM_DIALOG_GET_CLASS(obj) (G_TYPE_CHECK_GET_CLASS((obj), TRASH_TYPE_CONFIRM_DIALOG, TrashConfirmDialogClass))

typedef struct TrashConfirmDialogPrivate TrashConfirmDialogPrivate;

typedef struct {
    GtkRevealer parent_instance;

    TrashConfirmDialogPrivate *priv;
} TrashConfirmDialog;

typedef struct {
    GtkRevealerClass parent_class;

    void (*response)(TrashConfirmDialog *revealer, gint response_id);
} TrashConfirmDialogClass;

GType trash_confirm_dialog_get_type (void);

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
