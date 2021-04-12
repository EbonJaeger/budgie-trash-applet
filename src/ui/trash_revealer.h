#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_REVEALER (trash_revealer_get_type())
#define TRASH_REVEALER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TRASH_TYPE_REVEALER, TrashRevealer))
#define TRASH_IS_REVEALER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRASH_TYPE_REVEALER))

typedef struct _TrashRevealer TrashRevealer;
typedef struct _TrashRevealerClass TrashRevealerClass;

TrashRevealer *trash_revealer_new(void);
void trash_revealer_handle_clicked(GtkButton *sender, TrashRevealer *self);

void trash_revealer_set_text(TrashRevealer *self, gchar *text);

G_END_DECLS
