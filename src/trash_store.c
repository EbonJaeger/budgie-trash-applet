#include "trash_store.h"

enum {
    SIGNAL_TRASH_ADDED,
    SIGNAL_TRASH_REMOVED,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_SORT_MODE,
    LAST_PROP
};

static guint signals[LAST_SIGNAL];
static GParamSpec *props[LAST_PROP];

struct _TrashStore {
    GtkBox parent_instance;
    GFileMonitor *file_monitor;
    GSList *trashed_files;

    gchar *trash_path;
    TrashSortMode sort_mode;

    gboolean is_default;
    gboolean restoring;
    gint file_count;

    GtkWidget *header;
    GtkWidget *header_icon;
    GtkWidget *header_label;
    GtkWidget *reveal_icon;
    GtkWidget *delete_btn;
    GtkWidget *restore_btn;

    GtkWidget *file_revealer;
    GtkWidget *file_box;

    TrashConfirmDialog *dialog;
};

G_DEFINE_TYPE(TrashStore, trash_store, GTK_TYPE_BOX);

static void trash_store_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    TrashStore *self = TRASH_STORE(obj);

    switch (prop_id) {
        case PROP_SORT_MODE:
            g_value_set_enum(val, self->sort_mode);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_store_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    TrashStore *self = TRASH_STORE(obj);

    switch (prop_id) {
        case PROP_SORT_MODE:
            self->sort_mode = g_value_get_enum(val);
            gtk_list_box_invalidate_sort(GTK_LIST_BOX(self->file_box));
            g_object_notify_by_pspec(obj, props[PROP_SORT_MODE]);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_store_finalize(GObject *obj) {
    TrashStore *self = TRASH_STORE(obj);

    if (self->trashed_files) {
        // Not trying to free the widgets stored in the list because
        // I'm suspecting that they're already free'd by the time we
        // get here due to the container being destroyed by this point.
        g_slist_free(self->trashed_files);
    }

    g_free(self->trash_path);

    G_OBJECT_CLASS(trash_store_parent_class)->finalize(obj);
}

static void trash_store_class_init(TrashStoreClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS(klass);

    class->finalize = trash_store_finalize;
    class->get_property = trash_store_get_property;
    class->set_property = trash_store_set_property;

    // Signals

    signals[SIGNAL_TRASH_ADDED] = g_signal_newv(
        "trash-added",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
        NULL, NULL, NULL, NULL,
        G_TYPE_NONE,
        0,
        NULL
    );

    signals[SIGNAL_TRASH_REMOVED] = g_signal_newv(
        "trash-removed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
        NULL, NULL, NULL, NULL,
        G_TYPE_NONE,
        0,
        NULL
    );

    // Properties

    props[PROP_SORT_MODE] = g_param_spec_enum(
        "sort-mode",
        "Sort mode",
        "Set how trashed files should be sorted",
        TRASH_TYPE_SORT_MODE,
        TRASH_SORT_TYPE,
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE
    );

    g_object_class_install_properties(class, LAST_PROP, props);
}

static void set_button_sensitivity(TrashStore *self, gboolean sensitive) {
    gtk_widget_set_sensitive(self->delete_btn, sensitive);
    gtk_widget_set_sensitive(self->restore_btn, sensitive);
}

static void response_ok (TrashStore *self) {
    g_autoptr(GError) err = NULL;
    g_slist_foreach(self->trashed_files, self->restoring ? (GFunc) trash_item_restore : (GFunc) trash_item_delete, &err);

    if (err) {
        g_autofree gchar *body = g_strdup_printf("Error %s item from trash bin: %s", self->restoring ? "restoring" : "deleting", err->message);
        trash_notify_try_send("Trash Bin Error", body, "dialog-error-symbolic");
    } else {
        if (self->restoring) {
            trash_notify_try_send("Trash Restored", "All trashed files have been restored", NULL);
        } else {
            trash_notify_try_send("Trash Cleared", "All files cleared from the trash", NULL);
        }
    }

    set_button_sensitivity(self, TRUE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->dialog), FALSE);
}

static void dialog_response_cb (TrashConfirmDialog *dialog, gint response_id, TrashStore *self) {
    switch (response_id)
    {
    case GTK_RESPONSE_CANCEL:
        set_button_sensitivity(self, TRUE);
        gtk_revealer_set_reveal_child (GTK_REVEALER (dialog), FALSE);
        break;

    case GTK_RESPONSE_OK:
        response_ok (self);
        break;
    
    default:
        g_warning ("unknown response type: %d", response_id);
        break;
    }
}

static gboolean header_clicked(__attribute__((unused)) GtkWidget *sender, GdkEventButton *event, TrashStore *self) {
    gboolean ret = GDK_EVENT_PROPAGATE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (gtk_revealer_get_child_revealed(GTK_REVEALER(self->file_revealer))) {
                gtk_revealer_set_reveal_child(GTK_REVEALER(self->file_revealer), FALSE);
                gtk_image_set_from_icon_name(GTK_IMAGE(self->reveal_icon), "pan-start-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
            } else {
                gtk_revealer_set_reveal_child(GTK_REVEALER(self->file_revealer), TRUE);
                gtk_image_set_from_icon_name(GTK_IMAGE(self->reveal_icon), "pan-down-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
            }

            ret = GDK_EVENT_STOP;
            break;
        default:
            break;
    }

    return ret;
}

static void header_button_clicked(GtkButton *sender, TrashStore *self) {
    if (sender == GTK_BUTTON(self->delete_btn)) {
        self->restoring = FALSE;
        trash_confirm_dialog_show_message(self->dialog, "<b>Permanently delete all items in the trash bin?</b>", TRUE);
    } else {
        self->restoring = TRUE;
        trash_confirm_dialog_show_message(self->dialog, "<b>Restore all items from the trash bin?</b>", FALSE);
    }

    set_button_sensitivity(self, FALSE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->dialog), TRUE);
}

static void row_activated(__attribute__((unused)) GtkListBox *sender, GtkListBoxRow *row, __attribute__((unused)) TrashStore *self) {
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(row));
    trash_item_toggle_info_revealer(TRASH_ITEM(child));
}

static void apply_button_styles(TrashStore *self) {
    GtkStyleContext *delete_style = gtk_widget_get_style_context(self->delete_btn);
    gtk_style_context_add_class(delete_style, "flat");
    gtk_style_context_remove_class(delete_style, "button");
    GtkStyleContext *restore_style = gtk_widget_get_style_context(self->restore_btn);
    gtk_style_context_add_class(restore_style, "flat");
    gtk_style_context_remove_class(restore_style, "button");
}

static void check_empty(TrashStore *self) {
    GtkStyleContext *file_box_style = gtk_widget_get_style_context(self->file_box);

    if (self->file_count > 0) {
        gtk_style_context_remove_class(file_box_style, "empty");
        set_button_sensitivity(self, TRUE);
    } else {
        if (!gtk_style_context_has_class(file_box_style, "empty")) {
            gtk_style_context_add_class(file_box_style, "empty");
        }

        set_button_sensitivity(self, FALSE);
    }
}

static gint sort_rows(GtkListBoxRow *row1, GtkListBoxRow *row2, TrashStore *self) {
    TrashItem *item1 = TRASH_ITEM(gtk_bin_get_child(GTK_BIN(row1)));
    TrashItem *item2 = TRASH_ITEM(gtk_bin_get_child(GTK_BIN(row2)));

    switch (self->sort_mode) {
        case TRASH_SORT_A_Z:
            return trash_item_collate_by_name(item1, item2);
        case TRASH_SORT_Z_A:
            return trash_item_collate_by_name(item2, item1);
        case TRASH_SORT_DATE_ASCENDING:
            return trash_item_collate_by_date(item2, item1);
        case TRASH_SORT_DATE_DESCENDING:
            return trash_item_collate_by_date(item1, item2);
        case TRASH_SORT_TYPE:
            return trash_item_collate_by_type(item1, item2);
        default:
            g_critical("%s:%d: Unknown sort mode '%d', defaulting to by type", __BASE_FILE__, __LINE__, self->sort_mode);
            return trash_item_collate_by_type(item1, item2);
    }
}

static void trash_store_init(TrashStore *self) {
    self->restoring = FALSE;
    self->file_count = 0;
    self->trashed_files = NULL;

    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style, "trash-store-widget");
    gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);

    // Create our header box
    GtkWidget *header_event_box = gtk_event_box_new();
    GtkStyleContext *header_style = gtk_widget_get_style_context(header_event_box);
    gtk_style_context_add_class(header_style, "trash-store-header");
    g_signal_connect_object(header_event_box, "button-press-event", G_CALLBACK(header_clicked), self, 0);

    self->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(self->header, -1, 48);

    self->delete_btn = gtk_button_new_from_icon_name("list-remove-all-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(self->delete_btn, "Clear All");
    g_signal_connect_object(GTK_BUTTON(self->delete_btn), "clicked", G_CALLBACK(header_button_clicked), self, 0);
    self->restore_btn = gtk_button_new_from_icon_name("edit-undo-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(self->restore_btn, "Restore All");
    g_signal_connect_object(GTK_BUTTON(self->restore_btn), "clicked", G_CALLBACK(header_button_clicked), self, 0);
    gtk_box_pack_end(GTK_BOX(self->header), self->delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(self->header), self->restore_btn, FALSE, FALSE, 0);

    // Create our revealer object
    self->dialog = trash_confirm_dialog_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(self->dialog), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->dialog), FALSE);

    g_signal_connect (
        self->dialog,
        "response",
        G_CALLBACK (dialog_response_cb),
        self
    );

    self->file_revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(self->file_revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->file_revealer), TRUE);

    // Create our file list
    self->file_box = gtk_list_box_new();
    GtkStyleContext *file_box_style = gtk_widget_get_style_context(self->file_box);
    gtk_style_context_add_class(file_box_style, "trash-file-box");
    gtk_style_context_add_class(file_box_style, "empty");
    gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(self->file_box), TRUE);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(self->file_box), GTK_SELECTION_NONE);
    gtk_list_box_set_sort_func(GTK_LIST_BOX(self->file_box), (GtkListBoxSortFunc) sort_rows, self, NULL);

    g_signal_connect_object(self->file_box, "row-activated", G_CALLBACK(row_activated), self, 0);

    GtkWidget *placeholder = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkStyleContext *placeholder_style_context = gtk_widget_get_style_context(placeholder);
    gtk_style_context_add_class(placeholder_style_context, "dim-label");
    GtkWidget *label = gtk_label_new("<big>Trash bin is empty!</big>");
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_box_pack_start(GTK_BOX(placeholder), label, FALSE, FALSE, 0);
    gtk_widget_set_halign(placeholder, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(placeholder, GTK_ALIGN_CENTER);
    gtk_widget_show_all(placeholder);

    gtk_list_box_set_placeholder(GTK_LIST_BOX(self->file_box), placeholder);

    gtk_container_add(GTK_CONTAINER(self->file_revealer), self->file_box);

    // Pack ourselves up
    apply_button_styles(self);

    gtk_container_add(GTK_CONTAINER(header_event_box), self->header);

    gtk_box_pack_start(GTK_BOX(self), header_event_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(self), GTK_WIDGET(self->dialog), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->file_revealer, TRUE, TRUE, 0);
}

TrashStore *trash_store_new(gchar *drive_name, GIcon *icon, TrashSortMode mode) {
    TrashStore *self = g_object_new(TRASH_TYPE_STORE, "orientation", GTK_ORIENTATION_VERTICAL, "sort-mode", mode, NULL);
    self->trash_path = g_build_path(G_DIR_SEPARATOR_S, g_get_user_data_dir(), "Trash", "files", NULL);
    self->is_default = TRUE;

    self->header_icon = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_box_pack_start(GTK_BOX(self->header), self->header_icon, FALSE, FALSE, 0);

    self->header_label = gtk_label_new(g_strdup(drive_name));
    gtk_label_set_max_width_chars(GTK_LABEL(self->header_label), 30);
    gtk_label_set_ellipsize(GTK_LABEL(self->header_label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_halign(self->header_label, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(self->header_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start(GTK_BOX(self->header), self->header_label, TRUE, TRUE, 0);

    gtk_widget_set_tooltip_text(self->header, g_strdup(drive_name));

    self->reveal_icon = gtk_image_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_box_pack_start(GTK_BOX(self->header), self->reveal_icon, FALSE, FALSE, 0);

    gtk_widget_show_all(GTK_WIDGET(self));

    return self;
}

TrashStore *trash_store_new_with_path(gchar *drive_name,
                                      TrashSortMode mode,
                                      GIcon *icon,
                                      gchar *trash_path) {
    TrashStore *self = trash_store_new(drive_name, icon, mode);

    if (trash_utils_is_string_valid(self->trash_path)) {
        g_free(self->trash_path);
    }

    self->trash_path = g_strdup(trash_path);
    self->is_default = FALSE;

    return self;
}

/**
 * Constructs a URI from a `GFileInfo` and the `TrashStore` file location.
 * 
 * If the store is the default store, the URI will have the format `trash:///example.txt`.
 * Otherwise, in the case of mounts, the function will escape the path to the file because
 * that is what seems to happen when GLib(?) puts items in the `trash` URI scheme.
 */
static GUri *uri_for_file(TrashStore *self, const gchar *file_name) {
    g_autofree gchar *path = NULL;
    g_autoptr(GString) unescaped = NULL;
    g_autofree gchar *escaped = NULL;

    g_return_val_if_fail(TRASH_IS_STORE(self), NULL);

    if (!trash_utils_is_string_valid((gchar *) file_name)) {
        return NULL;
    }

    if (self->is_default) {
        path = g_strdup_printf("/%s", file_name);
    } else {
        /*
        * This abomination is because GLib does some really weird things with escaping characters
        * in paths for mounts in trash URIs. Specifically, forward slashes are back slashes and 
        * spaces are escaped. Then it get's even more special because for some reason at least part
        * of it gets escaped *before* that happens, because spaces end up as `%2520` (the % gets 
        * escaped). So, to get a valid URI to a trash file, we have to emulate that behavior.
        */

        unescaped = g_string_new(g_strdup_printf("%s\\%s", self->trash_path, file_name));
        g_string_replace(unescaped, "/", "\\", 0);
        g_string_replace(unescaped, " ", "%20", 0);
        escaped = g_uri_escape_string(unescaped->str, NULL, TRUE);
        path = g_strdup_printf("/%s", escaped);
    }

    return g_uri_build(
        G_URI_FLAGS_ENCODED,
        "trash",
        NULL,
        "",
        -1,
        path,
        NULL,
        NULL);
}

/**
 * Handles file events for this store's trash directory.
 * 
 * We handle G_FILE_MONITOR_EVENT_MOVED_IN, G_FILE_MONITOR_EVENT_MOVED_OUT, and
 * G_FILE_MONITOR_EVENT_DELETED events, adding and removing TrashItems
 * as needed.
 */
static void
handle_monitor_event(
    __attribute__((unused)) GFileMonitor *monitor,
    GFile *file,
    __attribute__((unused)) GFile *other_file,
    GFileMonitorEvent event_type,
    TrashStore *self
) {
    switch (event_type) {
        case G_FILE_MONITOR_EVENT_MOVED_IN: {
            g_autoptr(GUri) uri = uri_for_file(self, g_file_get_basename(file));
            g_return_if_fail(uri != NULL);

            g_autoptr(GError) err = NULL;
            TrashInfo *trash_info = trash_info_new(g_uri_to_string(uri), err);
            if (!TRASH_IS_INFO(trash_info)) {
                g_warning("%s:%d: Error making trash info for URI '%s': %s", __BASE_FILE__, __LINE__, g_uri_to_string(uri), err->message);
                return;
            }

            TrashItem *trash_item = trash_item_new(trash_info);
            g_return_if_fail(TRASH_IS_ITEM(trash_item));

            gtk_widget_show_all(GTK_WIDGET(trash_item));
            gtk_list_box_insert(GTK_LIST_BOX(self->file_box), GTK_WIDGET(trash_item), -1);
            self->trashed_files = g_slist_append(self->trashed_files, trash_item);
            self->file_count++;
            g_signal_emit(self, signals[SIGNAL_TRASH_ADDED], 0, NULL);

            check_empty(self);
            break;
        }
        case G_FILE_MONITOR_EVENT_MOVED_OUT:
        case G_FILE_MONITOR_EVENT_DELETED: {
            g_autofree gchar *file_name = g_file_get_basename(file);

            GSList *elem = g_slist_find_custom(self->trashed_files, file_name, (GCompareFunc) trash_item_has_name);
            TrashItem *trash_item = (TrashItem *) g_slist_nth_data(self->trashed_files,
                                                                   g_slist_position(self->trashed_files, elem));
            g_return_if_fail(TRASH_IS_ITEM(trash_item));

            GtkWidget *row = gtk_widget_get_parent(GTK_WIDGET(trash_item));
            gtk_container_remove(GTK_CONTAINER(self->file_box), row);

            self->file_count--;
            check_empty(self);
            self->trashed_files = g_slist_remove(self->trashed_files, trash_item);
            g_signal_emit(self, signals[SIGNAL_TRASH_REMOVED], 0, NULL);
            break;
        }
        default:
            break;
    }
}

void trash_store_start_monitor(TrashStore *self) {
    GFile *dir = g_file_new_for_path(self->trash_path);
    g_autoptr(GError) err = NULL;
    self->file_monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_WATCH_MOVES, NULL, &err);
    g_signal_connect_object(self->file_monitor, "changed", G_CALLBACK(handle_monitor_event), self, 0);
}

void trash_store_load_items(TrashStore *self, GError *err) {
    g_autoptr(GFile) trash_dir = g_file_new_for_path(self->trash_path);
    g_autoptr(GFileEnumerator) enumerator = g_file_enumerate_children(trash_dir,
                                                                      G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                                      G_FILE_QUERY_INFO_NONE,
                                                                      NULL,
                                                                      &err);
    if (!enumerator) {
        g_critical("%s:%d: Error getting file enumerator for trash files in '%s': %s", __BASE_FILE__, __LINE__, self->trash_path, err->message);
        return;
    }

    // Iterate over the directory's children and append each file name to a list
    g_autoptr(GFileInfo) current_file = NULL;
    while ((current_file = g_file_enumerator_next_file(enumerator, NULL, &err))) {
        g_autoptr(GUri) uri = uri_for_file(self, g_file_info_get_name(current_file));

        g_autoptr(GError) err = NULL;
        TrashInfo *trash_info = trash_info_new(g_uri_to_string(uri), err);
        if (!TRASH_IS_INFO(trash_info)) {
            g_warning("%s:%d: Error making trash info for URI '%s': %s", __BASE_FILE__, __LINE__, g_uri_to_string(uri), err->message);
            continue;
        }

        TrashItem *trash_item = trash_item_new(trash_info);
        if (!TRASH_IS_ITEM(trash_item)) {
            g_warning("%s:%d: Unable to make trash item for URI '%s'", __BASE_FILE__, __LINE__, g_uri_to_string(uri));
            continue;
        }

        gtk_widget_show_all(GTK_WIDGET(trash_item));
        gtk_list_box_insert(GTK_LIST_BOX(self->file_box), GTK_WIDGET(trash_item), -1);
        self->trashed_files = g_slist_append(self->trashed_files, trash_item);
        self->file_count++;
    }

    check_empty(self);
    g_file_enumerator_close(enumerator, NULL, NULL);
}

gint trash_store_get_count(TrashStore *self) {
    return self->file_count;
}
