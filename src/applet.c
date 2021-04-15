#define _GNU_SOURCE

#include "applet.h"
#include "trash_icon_button.h"
#include "trash_store.h"

struct _TrashAppletPrivate {
    BudgiePopoverManager *manager;
    GtkWidget *popover;
    TrashIconButton *icon_button;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED(TrashApplet, trash_applet, BUDGIE_TYPE_APPLET, 0, G_ADD_PRIVATE_DYNAMIC(TrashApplet))

/**
 * Handle cleanup of the applet.
 */
static void trash_applet_dispose(GObject *object) {
    G_OBJECT_CLASS(trash_applet_parent_class)->dispose(object);
}

/**
 * Register our popover with the Budgie popover manager.
 */
static void trash_applet_update_popovers(BudgieApplet *base, BudgiePopoverManager *manager) {
    TrashApplet *self = TRASH_APPLET(base);
    budgie_popover_manager_register_popover(manager,
                                            GTK_WIDGET(self->priv->icon_button),
                                            BUDGIE_POPOVER(self->priv->popover));
    self->priv->manager = manager;
}

/**
 * Initialize the Trash Applet class.
 */
static void trash_applet_class_init(TrashAppletClass *klazz) {
    GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

    obj_class->dispose = trash_applet_dispose;

    // Set our function to update popovers
    BUDGIE_APPLET_CLASS(klazz)->update_popovers = trash_applet_update_popovers;
}

/**
 * Apparently for cleanup that we don't have?
 */
static void trash_applet_class_finalize(__budgie_unused__ TrashAppletClass *klass) {
}

/**
 * Initialization of basic UI elements and loads our CSS
 * style stuff.
 */
static void trash_applet_init(TrashApplet *self) {
    // Create our 'private' struct
    self->priv = trash_applet_get_instance_private(self);

    // Load our CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/com/github/EbonJaeger/budgie-trash-applet/style/style.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Create our panel widget
    self->priv->icon_button = trash_icon_button_new();

    // Create our popover widget
    self->priv->popover = budgie_popover_new(GTK_WIDGET(self->priv->icon_button));
    g_object_set(self->priv->popover, "width-request", 300, NULL);
    trash_create_widgets(self, self->priv->popover);

    gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(self->priv->icon_button));

    g_signal_connect_object(GTK_BUTTON(self->priv->icon_button), "clicked", G_CALLBACK(trash_toggle_popover), self, 0);
    gtk_widget_show_all(GTK_WIDGET(self));
}

void trash_applet_init_gtype(GTypeModule *module) {
    trash_applet_register_type(module);
}

BudgieApplet *trash_applet_new(void) {
    return g_object_new(TRASH_TYPE_APPLET, NULL);
}

void trash_create_widgets(TrashApplet *self, GtkWidget *popover) {
    GtkWidget *view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create our popover header
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    g_object_set(header, "height-request", 32, NULL);
    GtkStyleContext *header_style = gtk_widget_get_style_context(header);
    gtk_style_context_add_class(header_style, "trash-applet-header");
    GtkWidget *header_label = gtk_label_new("Trash");
    gtk_box_pack_start(GTK_BOX(header), header_label, TRUE, TRUE, 0);

    // Create our scroller
    GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroller), 300);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroller), 300);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    // Create the listbox that the mounted drives will go into
    GtkWidget *drive_box = gtk_list_box_new();
    g_object_set(drive_box, "height-request", 300, NULL);
    gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(drive_box), TRUE);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(drive_box), GTK_SELECTION_NONE);
    GtkStyleContext *drive_box_style = gtk_widget_get_style_context(drive_box);
    gtk_style_context_add_class(drive_box_style, "trash-applet-list");
    gtk_container_add(GTK_CONTAINER(scroller), drive_box);

    // Create a dummy store for now to display
    TrashStore *default_store = trash_store_new("This PC", "drive-harddisk-symbolic");
    GError *err = NULL;
    trash_store_load_items(default_store, err);
    if (err) {
        g_critical("Error loading trash items for the default trash store: %s\n", err->message);
    }

    gtk_list_box_insert(GTK_LIST_BOX(drive_box), GTK_WIDGET(default_store), -1);

    gtk_box_pack_start(GTK_BOX(view), header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(view), scroller, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(popover), view);

    gtk_widget_show_all(view);
}

void trash_toggle_popover(GtkButton *sender, TrashApplet *self) {
    if (gtk_widget_is_visible(self->priv->popover)) {
        gtk_widget_hide(self->priv->popover);
    } else {
        budgie_popover_manager_show_popover(self->priv->manager, GTK_WIDGET(self->priv->icon_button));
    }
}
