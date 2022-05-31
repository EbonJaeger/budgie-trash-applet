#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum {
    TRASH_REVEALER_RESPONSE_OK = 0,
    TRASH_REVEALER_RESPONSE_CANCEL = 1
} TrashRevealerResponseType;

#define TRASH_TYPE_REVEALER (trash_revealer_get_type ())
#define TRASH_REVEALER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TRASH_TYPE_REVEALER, TrashRevealer))
#define TRASH_REVEALER_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST((obj), TRASH_TYPE_REVEALER, TrashRevealerClass))
#define TRASH_IS_REVEALER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRASH_TYPE_REVEALER))
#define TRASH_IS_REVEALER_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((obj), TRASH_TYPE_REVEALER))
#define TRASH_REVEALER_GET_CLASS(obj) (G_TYPE_CHECK_GET_CLASS((obj), TRASH_TYPE_REVEALER, TrashRevealerClass))

typedef struct TrashRevealerPrivate TrashRevealerPrivate;

typedef struct {
    GtkRevealer parent_instance;

    TrashRevealerPrivate *priv;
} TrashRevealer;

typedef struct {
    GtkRevealerClass parent_class;

    void (*response)(TrashRevealer *revealer, gint response_id);
} TrashRevealerClass;

GType trash_revealer_get_type (void);

/**
 * Create a new TrashRevealer widget.
 */
TrashRevealer *trash_revealer_new (void);

/**
 * Set the message for the revealer to show.
 * 
 * If `destructive` is set to true, the confirm button will have
 * the destructive CSS class applied to it.
 */
void trash_revealer_show_message (TrashRevealer *self, const gchar *message, gboolean destructive);

G_END_DECLS
