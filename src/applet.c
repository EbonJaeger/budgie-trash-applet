#define _GNU_SOURCE

#include "applet.h"
#include "ui/trash_icon_button.h"

G_DEFINE_DYNAMIC_TYPE_EXTENDED(TrashApplet, trash_applet, BUDGIE_TYPE_APPLET, 0, )

/**
 * Handle cleanup of the applet.
 */
static void trash_applet_dispose(GObject *object)
{
    G_OBJECT_CLASS(trash_applet_parent_class)->dispose(object);
}

/**
 * Initialize the  Trash Applet class.
 */
static void trash_applet_class_init(TrashAppletClass *klazz)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

    obj_class->dispose = trash_applet_dispose;
}

/**
 * Apparently for cleanup that we don't have?
 */
static void trash_applet_class_finalize(__budgie_unused__ TrashAppletClass *klass)
{
}

/**
 * Initialization of basic UI elements and loads our CSS
 * style stuff.
 */
static void trash_applet_init(TrashApplet *self)
{
    g_message("Trash Applet init");

    // Load our CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/com/github/EbonJaeger/budgie-trash-applet/style/style.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Create our panel widget
    TrashIconButton *icon_button = trash_icon_button_new();
    gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(icon_button));

    gtk_widget_show_all(GTK_WIDGET(self));
}

void trash_applet_init_gtype(GTypeModule *module)
{
    trash_applet_register_type(module);
}

BudgieApplet *trash_applet_new(void)
{
    return g_object_new(TRASH_TYPE_APPLET, NULL);
}
