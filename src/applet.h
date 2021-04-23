#pragma once

#include <budgie-desktop/applet.h>
#include <gtk/gtk.h>

#define __budgie_unused__ __attribute__((unused))

G_BEGIN_DECLS

typedef struct _TrashAppletPrivate TrashAppletPrivate;
typedef struct _TrashApplet TrashApplet;
typedef struct _TrashAppletClass TrashAppletClass;

#define TRASH_TYPE_APPLET trash_applet_get_type()
#define TRASH_APPLET(o) (G_TYPE_CHECK_INSTANCE_CAST((o), TRASH_TYPE_APPLET, TrashApplet))
#define TRASH_IS_APPLET(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), TRASH_TYPE_APPLET))
#define TRASH_APPLET_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), TRASH_TYPE_APPLET, TrashAppletClass))
#define TRASH_IS_APPLET_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), TRASH_TYPE_APPLET))
#define TRASH_APPLET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), TRASH_TYPE_APPLET, TrashAppletClass))

struct _TrashAppletClass {
    BudgieAppletClass parent_class;
};

struct _TrashApplet {
    BudgieApplet parent;
    TrashAppletPrivate *priv;
};

GType trash_applet_get_type(void);

/**
 * Public for the plugin to allow registration of types.
 */
void trash_applet_init_gtype(GTypeModule *module);

/**
 * Constructs a new  Trash Applet instance.
 */
BudgieApplet *trash_applet_new(void);

/**
 * Create our widgets to show in our popover.
 */
void trash_create_widgets(GtkWidget *popover);

/**
 * Shows our popover widget if it isn't currently visible, or hide
 * it if it is.
 */
void trash_toggle_popover(GtkButton *sender, TrashApplet *self);

G_END_DECLS
