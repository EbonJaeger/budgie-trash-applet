#include "applet.h"
#include "trash_popover.h"

#define _GNU_SOURCE

enum {
    PROP_0,
    PROP_APPLET_UUID,
    LAST_PROP
};

static GParamSpec *props[LAST_PROP];

struct _TrashAppletPrivate {
    BudgiePopoverManager *manager;
    GHashTable *mounts;

    gchar *uuid;

    GSettings *settings;

    GtkWidget *popover;
    GtkWidget *drive_box;
    TrashIconButton *icon_button;

    gint uid;
    GVolumeMonitor *volume_monitor;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED(TrashApplet, trash_applet, BUDGIE_TYPE_APPLET, 0, G_ADD_PRIVATE_DYNAMIC(TrashApplet))

/**
 * Handle cleanup of the applet.
 */
static void trash_applet_finalize(GObject *object) {
    TrashAppletPrivate *priv = trash_applet_get_instance_private(TRASH_APPLET(object));

    g_hash_table_destroy(priv->mounts);
    g_object_unref(priv->volume_monitor);
    g_object_unref(priv->settings);
    g_free(priv->uuid);

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
    GObjectClass *class = G_OBJECT_CLASS(klass);
    BudgieAppletClass *budgie_class = BUDGIE_APPLET_CLASS(klass);

    class->finalize = trash_applet_finalize;
    class->get_property = trash_applet_get_property;
    class->set_property = trash_applet_set_property;

    budgie_class->update_popovers = update_popovers;
    budgie_class->supports_settings = FALSE;

    props[PROP_APPLET_UUID] = g_param_spec_string(
        "uuid",
        "uuid",
        "The applet's UUID",
        NULL,
        G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE);

    g_object_class_install_properties(class, LAST_PROP, props);
}

/**
 * Handle cleanup of the applet class.
 */
static void trash_applet_class_finalize(__budgie_unused__ TrashAppletClass *klass) {
    notify_uninit();
}

static void setting_changed(GSettings *settings, gchar *key, TrashApplet *self) {
    if (strcmp(key, TRASH_SETTINGS_KEY_SORT_MODE) == 0) {
        // Set the sort mode everywhere
        TrashSortMode new_mode = g_settings_get_enum(settings, key);
        GHashTableIter iter;
        gpointer hash_key, value;

        g_hash_table_iter_init(&iter, self->priv->mounts);

        while (g_hash_table_iter_next(&iter, &hash_key, &value)) {
            g_assert(TRASH_IS_STORE(value));

            TrashStore *store = TRASH_STORE(value);
            trash_store_set_sort_mode(store, new_mode);
        }
    } else {
        g_critical("%s:%d: Unknown settings key '%s'", __BASE_FILE__, __LINE__, key);
    }
}

static void toggle_popover(__budgie_unused__ GtkButton *sender, TrashApplet *self) {
    if (gtk_widget_is_visible(self->priv->popover)) {
        gtk_widget_hide(self->priv->popover);
    } else {
        budgie_popover_manager_show_popover(self->priv->manager, GTK_WIDGET(self->priv->icon_button));
    }
}

/**
 * Iterate over all of the current trash stores, and update the icon
 * if there are items, or if there aren't.
 *
 * The iteration short-circuits as soon as it finds a trash store
 * that has items in it.
 */
static void maybe_update_icon(TrashApplet *self) {
    GHashTableIter iter;
    gpointer key, value;
    gboolean has_items = FALSE;

    g_hash_table_iter_init(&iter, self->priv->mounts);

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        g_assert(TRASH_IS_STORE(value));

        TrashStore *store = TRASH_STORE(value);
        gint count = trash_store_get_count(store);
        if (count > 0) {
            has_items = TRUE;
            break;
        }
    }

    if (has_items) {
        trash_icon_button_set_filled(self->priv->icon_button);
    } else {
        trash_icon_button_set_empty(self->priv->icon_button);
    }
}

static void trash_added(__budgie_unused__ TrashStore *store, TrashApplet *self) {
    maybe_update_icon(self);
}

static void trash_removed(__budgie_unused__ TrashStore *store, TrashApplet *self) {
    maybe_update_icon(self);
}

static TrashStore *create_store(TrashApplet *self, GMount *mount, GFile *mount_location, GFileInfo *info, GError *err) {
    TrashStore *store;
    g_autofree gchar *trash_path = NULL;

    trash_path = g_build_path(G_DIR_SEPARATOR_S, g_file_get_path(mount_location), g_file_info_get_name(info), "files", NULL);
    store = trash_store_new_with_path(g_mount_get_name(mount), g_settings_get_enum(self->priv->settings, TRASH_SETTINGS_KEY_SORT_MODE), g_mount_get_symbolic_icon(mount), g_strdup(trash_path));

    trash_store_get_items(store, err);
    g_return_val_if_fail(err == NULL, NULL);

    trash_store_start_monitor(store);
    g_signal_connect(TRASH_STORE(store), "trash-added", G_CALLBACK(trash_added), self);
    g_signal_connect(TRASH_STORE(store), "trash-removed", G_CALLBACK(trash_removed), self);

    return store;
}

static void add_mount(GMount *mount, TrashApplet *self) {
    TrashStore *store;
    g_autoptr(GError) err = NULL;
    g_autoptr(GError) inner_err = NULL;

    g_autoptr(GFile) location = g_mount_get_default_location(mount);

    // Calculate the length of the dir name we're looking for, with an extra
    // space for a NULL terminator. We use `snprintf` to count the length of
    // the UID so we can properly allocate space for it in the name string.
    gint name_length = (7 + snprintf(NULL, 0, "%i", self->priv->uid) + 1);
    g_autofree gchar *trash_dir_name = (gchar *) malloc(sizeof(gchar) * name_length);
    g_snprintf(trash_dir_name, name_length, ".Trash-%i", self->priv->uid);

    g_autoptr(GFileEnumerator) enumerator = g_file_enumerate_children(location,
                                                                      G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                                                      G_FILE_QUERY_INFO_NONE,
                                                                      NULL,
                                                                      &err);

    if (!enumerator) {
        g_critical("Error getting file enumerator for trash files in '%s': %s", g_file_get_path(location), err->message);
        return;
    }

    // Iterate over the items in the mount's default directory.
    // If a directory matches the name of an expected trash
    // directory, create a TrashStore for it and add it to the UI.
    g_autoptr(GFileInfo) current_file = NULL;
    while ((current_file = g_file_enumerator_next_file(enumerator, NULL, &err))) {
        // Check if the item is a directory with the matching trash
        // directory name.
        if (g_file_info_get_file_type(current_file) != G_FILE_TYPE_DIRECTORY ||
            strcmp(g_file_info_get_name(current_file), trash_dir_name) != 0) {
            continue;
        }

        store = create_store(self, mount, location, current_file, inner_err);

        if (store == NULL) {
            g_warning("Error creating TrashStore: %s", inner_err->message);
            break;
        }

        gtk_list_box_insert(GTK_LIST_BOX(self->priv->drive_box), GTK_WIDGET(store), -1);
        g_hash_table_insert(self->priv->mounts, g_strdup(g_mount_get_name(mount)), store);
        break;
    }

    g_file_enumerator_close(enumerator, NULL, NULL);

    // Update the icon if needed
    maybe_update_icon(self);
}

static void mount_added(__budgie_unused__ GVolumeMonitor *monitor, GMount *mount, TrashApplet *self) {
    add_mount(mount, self);
}

static void mount_removed(__budgie_unused__ GVolumeMonitor *monitor, GMount *mount, TrashApplet *self) {
    g_autofree gchar *mount_name = g_mount_get_name(mount);
    TrashStore *store = (TrashStore *) g_hash_table_lookup(self->priv->mounts, mount_name);
    g_return_if_fail(TRASH_IS_STORE(store));

    GtkWidget *row = gtk_widget_get_parent(GTK_WIDGET(store));
    gtk_container_remove(GTK_CONTAINER(self->priv->drive_box), row);
    g_hash_table_remove(self->priv->mounts, mount_name);
    maybe_update_icon(self);
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

    self->priv->mounts = g_hash_table_new(g_str_hash, g_str_equal);
    self->priv->settings = g_settings_new(TRASH_SETTINGS_SCHEMA_ID);
    g_signal_connect_object(self->priv->settings, "changed", G_CALLBACK(setting_changed), self, 0);

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

    // Create our popover widget
    self->priv->popover = budgie_popover_new(GTK_WIDGET(self->priv->icon_button));
    TrashPopover *popover_body = trash_popover_new();
    gtk_container_add(GTK_CONTAINER(self->priv->popover), GTK_WIDGET(popover_body));

    gtk_widget_show_all(GTK_WIDGET(self));

    // Register notifications
    notify_init("com.github.EbonJaeger.budgie-trash-applet");

    // Setup our volume monitor to get trashbins for
    // removable drives
    self->priv->uid = getuid();
    self->priv->volume_monitor = g_volume_monitor_get();
    g_autoptr(GList) mounts = g_volume_monitor_get_mounts(self->priv->volume_monitor);
    g_list_foreach(mounts, (GFunc) add_mount, self);

    g_signal_connect_object(self->priv->volume_monitor, "mount-added", G_CALLBACK(mount_added), self, G_CONNECT_AFTER);
    g_signal_connect_object(self->priv->volume_monitor, "mount-removed", G_CALLBACK(mount_removed), self, G_CONNECT_AFTER);

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
