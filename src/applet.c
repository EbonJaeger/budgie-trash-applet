#define _GNU_SOURCE

#include "applet.h"
#include "ui/trash_icon_button.h"

struct _TrashAppletPrivate
{
    BudgiePopoverManager *manager;
    GtkWidget *popover;
    TrashIconButton *icon_button;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED(TrashApplet, trash_applet, BUDGIE_TYPE_APPLET, 0, G_ADD_PRIVATE_DYNAMIC(TrashApplet))

/**
 * Handle cleanup of the applet.
 */
static void trash_applet_dispose(GObject *object)
{
    G_OBJECT_CLASS(trash_applet_parent_class)->dispose(object);
}

/**
 * Register our popover with the Budgie popover manager.
 */
static void trash_applet_update_popovers(BudgieApplet *base, BudgiePopoverManager *manager)
{
    TrashApplet *self = TRASH_APPLET(base);
    budgie_popover_manager_register_popover(manager,
                                            GTK_WIDGET(self->priv->icon_button),
                                            BUDGIE_POPOVER(self->priv->popover));
    self->priv->manager = manager;
}

/**
 * Initialize the Trash Applet class.
 */
static void trash_applet_class_init(TrashAppletClass *klazz)
{
    GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

    obj_class->dispose = trash_applet_dispose;

    // Set our function to update popovers
    BUDGIE_APPLET_CLASS(klazz)->update_popovers = trash_applet_update_popovers;
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
    // Create our 'private' struct
    self->priv = trash_applet_get_instance_private(self);

    // Load our CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/com/github/EbonJaeger/budgie-trash-applet/style/style.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Create our panel widget
    TrashIconButton *icon_button = trash_icon_button_new();
    self->priv->icon_button = icon_button;

    // Create our popover widget
    GtkWidget *popover = budgie_popover_new(GTK_WIDGET(icon_button));
    gtk_widget_set_size_request(popover, 300, 400);
    self->priv->popover = popover;

    gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(icon_button));

    g_signal_connect_object(GTK_BUTTON(icon_button), "clicked", G_CALLBACK(trash_toggle_popover), self, 0);
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

void trash_toggle_popover(GtkButton *sender, TrashApplet *self)
{
    if (gtk_widget_is_visible(self->priv->popover))
    {
        gtk_widget_hide(self->priv->popover);
    }
    else
    {
        budgie_popover_manager_show_popover(self->priv->manager, GTK_WIDGET(self->priv->icon_button));
    }
}
