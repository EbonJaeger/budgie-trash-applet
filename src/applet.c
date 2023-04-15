#include "applet.h"

#define _GNU_SOURCE

enum {
    PROP_0,
    PROP_APPLET_UUID,
    N_PROPS
};

static GParamSpec *props[N_PROPS];

struct _TrashAppletPrivate {
    BudgiePopoverManager *manager;

    gchar *uuid;

    GtkWidget *popover;
    TrashIconButton *icon_button;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED(TrashApplet, trash_applet, BUDGIE_TYPE_APPLET, 0, G_ADD_PRIVATE_DYNAMIC(TrashApplet))

static void trash_applet_constructed(GObject *object) {
    TrashApplet *self = TRASH_APPLET(object);
    TrashPopover *popover_body;

    // Set our settings schema and prefix
    g_object_set(self,
                 "settings-schema", "com.solus-project.budgie-trash-applet",
                 "settings-prefix", "/com/solus-project/budgie-panel/instance/budgie-trash-applet",
                 NULL);
    self->settings = budgie_applet_get_applet_settings(BUDGIE_APPLET(self), self->priv->uuid);

    // Create our popover widget
    self->priv->popover = budgie_popover_new(GTK_WIDGET(self->priv->icon_button));
    popover_body = trash_popover_new(self->settings);
    gtk_container_add(GTK_CONTAINER(self->priv->popover), GTK_WIDGET(popover_body));

    G_OBJECT_CLASS(trash_applet_parent_class)->constructed(object);
}

/**
 * Handle cleanup of the applet.
 */
static void trash_applet_finalize(GObject *object) {
    TrashApplet *self;
    TrashAppletPrivate *priv;

    self = TRASH_APPLET(object);
    priv = trash_applet_get_instance_private(self);

    g_free(priv->uuid);

    if (self->settings) {
        g_object_unref(self->settings);
    }

    G_OBJECT_CLASS(trash_applet_parent_class)->finalize(object);
}

static void trash_applet_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    TrashApplet *self = TRASH_APPLET(obj);

    switch (prop_id) {
        case PROP_APPLET_UUID:
            g_value_set_string(val, self->priv->uuid);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_applet_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    TrashApplet *self = TRASH_APPLET(obj);

    switch (prop_id) {
        case PROP_APPLET_UUID:
            trash_applet_update_uuid(self, g_value_get_string(val));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static GtkWidget *trash_applet_get_settings_ui(BudgieApplet *base) {
    TrashApplet *self = TRASH_APPLET(base);
    TrashSettings *trash_settings;

    trash_settings = trash_settings_new(self->settings);
    g_object_ref_sink(trash_settings);

    return GTK_WIDGET(trash_settings);
}

static gboolean trash_applet_supports_settings(BudgieApplet *base) {
    (void) base;
    return TRUE;
}

/**
 * Register our popover with the Budgie popover manager.
 */
static void update_popovers(BudgieApplet *base, BudgiePopoverManager *manager) {
    TrashApplet *self = TRASH_APPLET(base);
    budgie_popover_manager_register_popover(manager,
                                            GTK_WIDGET(self->priv->icon_button),
                                            BUDGIE_POPOVER(self->priv->popover));
    self->priv->manager = manager;
}

/**
 * Initialize the Trash Applet class.
 */
static void trash_applet_class_init(TrashAppletClass *klass) {
    GObjectClass *class;
    BudgieAppletClass *budgie_class;

    class = G_OBJECT_CLASS(klass);
    budgie_class = BUDGIE_APPLET_CLASS(klass);

    class->constructed = trash_applet_constructed;
    class->finalize = trash_applet_finalize;
    class->get_property = trash_applet_get_property;
    class->set_property = trash_applet_set_property;

    budgie_class->update_popovers = update_popovers;
    budgie_class->supports_settings = trash_applet_supports_settings;
    budgie_class->get_settings_ui = trash_applet_get_settings_ui;

    props[PROP_APPLET_UUID] = g_param_spec_string(
        "uuid",
        "uuid",
        "The applet's UUID",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(class, N_PROPS, props);
}

/**
 * Handle cleanup of the applet class.
 */
static void trash_applet_class_finalize(__budgie_unused__ TrashAppletClass *klass) {
    notify_uninit();
}

static void toggle_popover(__budgie_unused__ GtkButton *sender, TrashApplet *self) {
    if (gtk_widget_is_visible(self->priv->popover)) {
        gtk_widget_hide(self->priv->popover);
    } else {
        budgie_popover_manager_show_popover(self->priv->manager, GTK_WIDGET(self->priv->icon_button));
    }
}

static void drag_data_received(
    __budgie_unused__ TrashApplet *self,
    GdkDragContext *context,
    __budgie_unused__ gint x,
    __budgie_unused__ gint y,
    GtkSelectionData *data,
    guint info,
    guint time) {
    g_return_if_fail(info == 0);

    g_autofree gchar *uri = g_strdup((gchar *) gtk_selection_data_get_data(data));
    g_autofree gchar *unescaped = NULL;
    g_autoptr(GFile) file = NULL;
    g_autoptr(GError) err = NULL;

    if (g_str_has_prefix(uri, "file://")) {
        unescaped = g_uri_unescape_string(uri, NULL);
        g_strstrip(unescaped); // Make sure there's nothing silly like a trailing newline
        file = g_file_new_for_uri(unescaped);

        if (!g_file_trash(file, NULL, &err)) {
            trash_notify_try_send("Error Trashing File", err->message, "dialog-error-symbolic");
            g_critical("%s:%d: Error moving file to trash: %s", __BASE_FILE__, __LINE__, err->message);
            return;
        }
    }

    gtk_drag_finish(context, TRUE, TRUE, time);
}

// static GtkWidget *create_main_view(TrashApplet *self, TrashSortMode sort_mode) {
//     GtkWidget *main_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

//     // Create our popover header
//     GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
//     GtkStyleContext *header_style = gtk_widget_get_style_context(header);
//     gtk_style_context_add_class(header_style, "trash-applet-header");
//     GtkWidget *header_label = gtk_label_new("Trash");
//     GtkStyleContext *header_label_style = gtk_widget_get_style_context(header_label);
//     gtk_style_context_add_class(header_label_style, "title");
//     gtk_box_pack_start(GTK_BOX(header), header_label, TRUE, TRUE, 0);

//     // Create our scroller
//     GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
//     gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroller), 300);
//     gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroller), 300);
//     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

//     // Create the listbox that the mounted drives will go into
//     self->priv->drive_box = gtk_list_box_new();
//     gtk_widget_set_size_request(self->priv->drive_box, -1, 300);
//     gtk_list_box_set_selection_mode(GTK_LIST_BOX(self->priv->drive_box), GTK_SELECTION_NONE);
//     GtkStyleContext *drive_box_style = gtk_widget_get_style_context(self->priv->drive_box);
//     gtk_style_context_add_class(drive_box_style, "trash-applet-list");
//     gtk_container_add(GTK_CONTAINER(scroller), self->priv->drive_box);

//     // Create the trash store widgets
//     TrashStore *default_store = trash_store_new("This PC", g_icon_new_for_string("drive-harddisk-symbolic", NULL), sort_mode);
//     g_autoptr(GError) err = NULL;
//     trash_store_load_items(default_store, err);
//     if (err) {
//         g_critical("Error loading trash items for the default trash store: %s", err->message);
//     }

//     trash_store_start_monitor(default_store);
//     g_signal_connect_object(TRASH_STORE(default_store), "trash-added", G_CALLBACK(trash_added), self, 0);
//     g_signal_connect_object(TRASH_STORE(default_store), "trash-removed", G_CALLBACK(trash_removed), self, 0);

//     g_hash_table_insert(self->priv->mounts, "This PC", default_store);
//     gtk_list_box_insert(GTK_LIST_BOX(self->priv->drive_box), GTK_WIDGET(default_store), -1);

//     // Footer
//     GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
//     GtkStyleContext *footer_style = gtk_widget_get_style_context(footer);
//     gtk_style_context_add_class(footer_style, "trash-applet-footer");

//     self->priv->settings_button = gtk_button_new_from_icon_name("emblem-system-symbolic", GTK_ICON_SIZE_BUTTON);
//     gtk_widget_set_tooltip_text(self->priv->settings_button, "Settings");
//     GtkStyleContext *settings_button_context = gtk_widget_get_style_context(self->priv->settings_button);
//     gtk_style_context_add_class(settings_button_context, "flat");
//     gtk_style_context_remove_class(settings_button_context, "button");
//     gtk_box_pack_start(GTK_BOX(footer), self->priv->settings_button, TRUE, FALSE, 0);
//     g_signal_connect_object(GTK_BUTTON(self->priv->settings_button), "clicked", G_CALLBACK(trash_settings_clicked), self, 0);

//     // Pack it all up
//     gtk_box_pack_start(GTK_BOX(main_view), header, FALSE, FALSE, 0);
//     gtk_box_pack_start(GTK_BOX(main_view), scroller, TRUE, TRUE, 0);
//     gtk_box_pack_end(GTK_BOX(main_view), footer, FALSE, FALSE, 0);

//     // Show everything
//     gtk_widget_show_all(main_view);
//     maybe_update_icon(self);

//     return main_view;
// }

/**
 * Initialization of basic UI elements and loads our CSS
 * style stuff.
 */
static void trash_applet_init(TrashApplet *self) {
    // Create our 'private' struct
    self->priv = trash_applet_get_instance_private(self);

    // Load our CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/com/github/EbonJaeger/budgie-trash-applet/style.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Create our panel widget
    self->priv->icon_button = trash_icon_button_new();
    g_signal_connect_object(GTK_BUTTON(self->priv->icon_button), "clicked", G_CALLBACK(toggle_popover), self, 0);
    gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(self->priv->icon_button));

    gtk_widget_show_all(GTK_WIDGET(self));

    // Register notifications
    notify_init("com.github.EbonJaeger.budgie-trash-applet");

    // Setup drag and drop to trash files
    gtk_drag_dest_set(GTK_WIDGET(self),
                      GTK_DEST_DEFAULT_ALL,
                      gtk_target_entry_new("text/uri-list", 0, 0),
                      1,
                      GDK_ACTION_COPY);

    g_signal_connect_object(self, "drag-data-received", G_CALLBACK(drag_data_received), self, 0);
}

void trash_applet_init_gtype(GTypeModule *module) {
    trash_applet_register_type(module);
}

TrashApplet *trash_applet_new(const gchar *uuid) {
    return g_object_new(TRASH_TYPE_APPLET, "uuid", uuid, NULL);
}

void trash_applet_update_uuid(TrashApplet *self, const gchar *value) {
    g_return_if_fail(TRASH_IS_APPLET(self));

    if (!trash_utils_is_string_valid((char *) value)) {
        return;
    }

    if (trash_utils_is_string_valid(self->priv->uuid)) {
        g_free(self->priv->uuid);
    }

    self->priv->uuid = g_strdup(value);
    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_APPLET_UUID]);
}
