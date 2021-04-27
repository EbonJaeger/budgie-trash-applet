#include "trash_store.h"
#include "applet.h"
#include "notify.h"
#include "trash_info.h"
#include "utils.h"

enum {
    PROP_EXP_0,
    PROP_DRIVE_NAME,
    PROP_ICON,
    PROP_PATH_PREFIX,
    PROP_TRASH_PATH,
    PROP_TRASHINFO_PATH,
    N_EXP_PROPERTIES
};

static GParamSpec *store_props[N_EXP_PROPERTIES] = {
    NULL,
};

struct _TrashStore {
    GtkBox parent_instance;
    GFileMonitor *file_monitor;
    GSList *trashed_files;

    gchar *path_prefix;
    gchar *trash_path;
    gchar *trashinfo_path;

    gchar *drive_name;
    GIcon *icon;
    gboolean restoring;
    gint file_count;

    GtkWidget *header;
    GtkWidget *header_icon;
    GtkWidget *header_label;
    GtkWidget *delete_btn;
    GtkWidget *restore_btn;

    GtkWidget *file_box;

    TrashRevealer *revealer;
};

struct _TrashStoreClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(TrashStore, trash_store, GTK_TYPE_BOX);

static void trash_store_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec);
static void trash_store_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec);

static void trash_store_class_init(TrashStoreClass *klazz) {
    GObjectClass *class = G_OBJECT_CLASS(klazz);
    class->get_property = trash_store_get_property;
    class->set_property = trash_store_set_property;

    store_props[PROP_DRIVE_NAME] = g_param_spec_string(
        "drive-name",
        "Drive Name",
        "Name of the drive where the trash bin is located",
        "This PC",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    store_props[PROP_ICON] = g_param_spec_gtype(
        "icon",
        "Icon",
        "Icon to use for this drive",
        G_TYPE_NONE,
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    store_props[PROP_PATH_PREFIX] = g_param_spec_string(
        "path-prefix",
        "Path prefix",
        "Path prefix to prepend to trash item paths in this TrashStore",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    store_props[PROP_TRASH_PATH] = g_param_spec_string(
        "trash-path",
        "Trash path",
        "Path to the directory where trashed files are",
        g_build_path(G_DIR_SEPARATOR_S, g_get_user_data_dir(), "Trash", "files", NULL),
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    store_props[PROP_TRASHINFO_PATH] = g_param_spec_string(
        "trashinfo-path",
        "Trashinfo path",
        "Path to the directory where the trashinfo files are",
        g_build_path(G_DIR_SEPARATOR_S, g_get_user_data_dir(), "Trash", "info", NULL),
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    g_object_class_install_properties(class, N_EXP_PROPERTIES, store_props);
}

static void trash_store_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    TrashStore *self = TRASH_STORE(obj);

    switch (prop_id) {
        case PROP_DRIVE_NAME:
            g_value_set_string(val, self->drive_name);
            break;
        case PROP_ICON:
            g_value_set_gtype(val, (GType) self->icon);
            break;
        case PROP_PATH_PREFIX:
            g_value_set_string(val, self->path_prefix);
            break;
        case PROP_TRASH_PATH:
            g_value_set_string(val, self->trash_path);
            break;
        case PROP_TRASHINFO_PATH:
            g_value_set_string(val, self->trashinfo_path);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_store_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    TrashStore *self = TRASH_STORE(obj);

    switch (prop_id) {
        case PROP_DRIVE_NAME:
            g_return_if_fail(GTK_IS_WIDGET(self->header));
            trash_store_set_drive_name(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_ICON:
            g_return_if_fail(GTK_IS_WIDGET(self->header));
            trash_store_set_icon(self, G_ICON(g_value_get_gtype(val)));
            break;
        case PROP_PATH_PREFIX:
            trash_store_set_path_prefix(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_TRASH_PATH:
            trash_store_set_trash_path(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_TRASHINFO_PATH:
            trash_store_set_trashinfo_path(self, g_strdup(g_value_get_string(val)));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
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
    self->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkStyleContext *header_style = gtk_widget_get_style_context(self->header);
    gtk_style_context_add_class(header_style, "trash-store-widget");
    g_object_set(G_OBJECT(self->header), "height-request", 48, NULL);

    self->delete_btn = gtk_button_new_from_icon_name("list-remove-all-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(self->delete_btn, "Clear All");
    g_signal_connect_object(GTK_BUTTON(self->delete_btn), "clicked", G_CALLBACK(trash_store_handle_header_btn_clicked), self, 0);
    self->restore_btn = gtk_button_new_from_icon_name("edit-undo-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(self->restore_btn, "Restore All");
    g_signal_connect_object(GTK_BUTTON(self->restore_btn), "clicked", G_CALLBACK(trash_store_handle_header_btn_clicked), self, 0);
    gtk_box_pack_end(GTK_BOX(self->header), self->delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(self->header), self->restore_btn, FALSE, FALSE, 0);

    // Create our revealer object
    self->revealer = trash_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(self->revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->revealer), FALSE);

    g_signal_connect_object(GTK_REVEALER(self->revealer), "cancel-clicked", G_CALLBACK(trash_store_handle_cancel_clicked), self, 0);
    g_signal_connect_object(GTK_REVEALER(self->revealer), "confirm-clicked", G_CALLBACK(trash_store_handle_confirm_clicked), self, 0);

    // Create our file list
    self->file_box = gtk_list_box_new();
    GtkStyleContext *file_box_style = gtk_widget_get_style_context(self->file_box);
    gtk_style_context_add_class(file_box_style, "trash-file-box");
    gtk_style_context_add_class(file_box_style, "empty");
    gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(self->file_box), TRUE);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(self->file_box), GTK_SELECTION_NONE);
    gtk_list_box_set_sort_func(GTK_LIST_BOX(self->file_box), trash_store_sort, self, NULL);

    g_signal_connect_object(self->file_box, "row-activated", G_CALLBACK(trash_store_handle_row_activated), self, 0);

    // Pack ourselves up
    trash_store_apply_button_styles(self);
    gtk_box_pack_start(GTK_BOX(self), self->header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(self), GTK_WIDGET(self->revealer), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->file_box, TRUE, TRUE, 0);
}

TrashStore *trash_store_new(gchar *drive_name) {
    return g_object_new(TRASH_TYPE_STORE,
                        "orientation", GTK_ORIENTATION_VERTICAL,
                        "drive-name", drive_name,
                        "icon", g_icon_new_for_string("drive-harddisk-symbolic", NULL),
                        NULL);
}

TrashStore *trash_store_new_with_extras(gchar *drive_name, GIcon *icon, gchar *path_prefix, gchar *trash_path, gchar *trashinfo_path) {
    return g_object_new(TRASH_TYPE_STORE,
                        "orientation", GTK_ORIENTATION_VERTICAL,
                        "drive-name", drive_name,
                        "icon", icon,
                        "path-prefix", path_prefix,
                        "trash-path", trash_path,
                        "trashinfo-path", trashinfo_path,
                        NULL);
}

void trash_store_apply_button_styles(TrashStore *self) {
    GtkStyleContext *delete_style = gtk_widget_get_style_context(self->delete_btn);
    gtk_style_context_add_class(delete_style, "flat");
    gtk_style_context_remove_class(delete_style, "button");
    GtkStyleContext *restore_style = gtk_widget_get_style_context(self->restore_btn);
    gtk_style_context_add_class(restore_style, "flat");
    gtk_style_context_remove_class(restore_style, "button");
}

void trash_store_set_drive_name(TrashStore *self, gchar *drive_name) {
    gchar *name_clone = g_strdup(drive_name);

    if (name_clone == NULL || strcmp(name_clone, "") == 0) {
        return;
    }

    if (!GTK_IS_WIDGET(self->header)) {
        return;
    }

    // Free existing text if it is different
    if ((self->drive_name != NULL) && strcmp(self->drive_name, name_clone) != 0) {
        g_free(self->drive_name);
    }

    self->drive_name = name_clone;

    // If we already have a label, just set new text. Otherwise,
    // Create a new label.
    if (GTK_IS_LABEL(self->header_label)) {
        gtk_label_set_text(GTK_LABEL(self->header_label), self->drive_name);
    } else {
        self->header_label = gtk_label_new(self->drive_name);
        gtk_label_set_max_width_chars(GTK_LABEL(self->header_label), 30);
        gtk_label_set_ellipsize(GTK_LABEL(self->header_label), PANGO_ELLIPSIZE_END);
        gtk_widget_set_halign(self->header_label, GTK_ALIGN_START);
        gtk_label_set_justify(GTK_LABEL(self->header_label), GTK_JUSTIFY_LEFT);
        gtk_box_pack_end(GTK_BOX(self->header), self->header_label, TRUE, TRUE, 0);
    }

    gtk_widget_set_tooltip_text(self->header, self->drive_name);

    g_object_notify_by_pspec(G_OBJECT(self), store_props[PROP_DRIVE_NAME]);
}

void trash_store_set_icon(TrashStore *self, GIcon *icon) {
    if (!GTK_IS_WIDGET(self->header)) {
        return;
    }

    if (g_icon_equal(self->icon, icon)) {
        return;
    } else {
        if (self->icon) {
            g_free(self->icon);
        }
    }

    self->icon = icon;

    // If we already have an icon set, change it. Else, make a new one and prepend it
    // to our header.
    if (GTK_IS_IMAGE(self->header_icon)) {
        gtk_image_set_from_gicon(GTK_IMAGE(self->header_icon), self->icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
    } else {
        self->header_icon = gtk_image_new_from_gicon(self->icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_box_pack_start(GTK_BOX(self->header), self->header_icon, FALSE, FALSE, 10);
    }

    g_object_notify_by_pspec(G_OBJECT(self), store_props[PROP_ICON]);
}

void trash_store_set_path_prefix(TrashStore *self, gchar *path_prefix) {
    gchar *path_clone = g_strdup(path_prefix);

    if (path_clone == NULL || strcmp(path_clone, "") == 0) {
        return;
    }

    // Free existing text if it is different
    if ((self->path_prefix != NULL) && strcmp(self->path_prefix, path_clone) != 0) {
        g_free(self->path_prefix);
    }

    self->path_prefix = path_clone;

    g_object_notify_by_pspec(G_OBJECT(self), store_props[PROP_PATH_PREFIX]);
}

void trash_store_set_trash_path(TrashStore *self, gchar *trash_path) {
    gchar *path_clone = g_strdup(trash_path);

    if (path_clone == NULL || strcmp(path_clone, "") == 0) {
        return;
    }

    // Free existing text if it is different
    if ((self->trash_path != NULL) && strcmp(self->trash_path, path_clone) != 0) {
        if (self->file_monitor) {
            g_file_monitor_cancel(G_FILE_MONITOR(self->file_monitor));
            g_object_unref(self->file_monitor);
        }
        g_free(self->trash_path);
    }

    self->trash_path = path_clone;

    // Set up our file monitor
    GFile *dir = g_file_new_for_path(self->trash_path);
    g_autoptr(GError) err = NULL;
    self->file_monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_WATCH_MOVES, NULL, &err);
    g_signal_connect_object(self->file_monitor, "changed", G_CALLBACK(trash_store_handle_monitor_event), self, 0);

    g_object_notify_by_pspec(G_OBJECT(self), store_props[PROP_TRASH_PATH]);
}

void trash_store_set_trashinfo_path(TrashStore *self, gchar *trashinfo_path) {
    gchar *path_clone = g_strdup(trashinfo_path);

    if (path_clone == NULL || strcmp(path_clone, "") == 0) {
        return;
    }

    // Free existing text if it is different
    if ((self->trashinfo_path != NULL) && strcmp(self->trashinfo_path, path_clone) != 0) {
        g_free(self->trashinfo_path);
    }

    self->trashinfo_path = path_clone;

    g_object_notify_by_pspec(G_OBJECT(self), store_props[PROP_TRASHINFO_PATH]);
}

void trash_store_set_btns_sensitive(TrashStore *self, gboolean sensitive) {
    gtk_widget_set_sensitive(self->delete_btn, sensitive);
    gtk_widget_set_sensitive(self->restore_btn, sensitive);
}

void trash_store_check_empty(TrashStore *self) {
    GtkStyleContext *file_box_style = gtk_widget_get_style_context(self->file_box);

    if (self->file_count > 0) {
        gtk_style_context_remove_class(file_box_style, "empty");
        trash_store_set_btns_sensitive(self, TRUE);
    } else {
        if (!gtk_style_context_has_class(file_box_style, "empty")) {
            gtk_style_context_add_class(file_box_style, "empty");
        }

        trash_store_set_btns_sensitive(self, FALSE);
    }
}

void trash_store_handle_header_btn_clicked(GtkButton *sender, TrashStore *self) {
    if (sender == GTK_BUTTON(self->delete_btn)) {
        self->restoring = FALSE;
        trash_revealer_set_text(self->revealer, "<b>Permanently delete all items in the trash bin?</b>");
    } else {
        self->restoring = TRUE;
        trash_revealer_set_text(self->revealer, "<b>Restore all items from the trash bin?</b>");
    }

    trash_store_set_btns_sensitive(self, FALSE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->revealer), TRUE);
}

void trash_store_handle_cancel_clicked(__budgie_unused__ TrashRevealer *sender, TrashStore *self) {
    trash_store_set_btns_sensitive(self, TRUE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->revealer), FALSE);
}

void trash_store_handle_confirm_clicked(__budgie_unused__ TrashRevealer *sender, TrashStore *self) {
    g_autoptr(GError) err = NULL;
    g_slist_foreach(self->trashed_files, self->restoring ? (GFunc) trash_item_restore : (GFunc) trash_item_delete, &err);
    if (err) {
        trash_notify_try_send("Trash Bin Error", err->message, "dialog-error-symbolic");
    }

    if (self->restoring) {
        trash_notify_try_send("Trash Restored", "All trashed files have been restored", NULL);
    } else {
        trash_notify_try_send("Trash Cleared", "All files cleared from the trash", NULL);
    }

    trash_store_set_btns_sensitive(self, TRUE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->revealer), FALSE);
}

void trash_store_handle_row_activated(__budgie_unused__ GtkListBox *sender, GtkListBoxRow *row, __budgie_unused__ TrashStore *self) {
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(row));
    trash_item_toggle_info_revealer(TRASH_ITEM(child));
}

void trash_store_handle_monitor_event(__budgie_unused__ GFileMonitor *monitor,
                                      GFile *file,
                                      __budgie_unused__ GFile *other_file,
                                      GFileMonitorEvent event_type,
                                      TrashStore *self) {
    switch (event_type) {
        case G_FILE_MONITOR_EVENT_MOVED_IN: {
            g_autofree gchar *attributes = g_strconcat(G_FILE_ATTRIBUTE_STANDARD_NAME, ",",
                                                       G_FILE_ATTRIBUTE_STANDARD_ICON, ",",
                                                       G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                                       NULL);
            GFileInfo *file_info = g_file_query_info(file, attributes, G_FILE_QUERY_INFO_NONE, NULL, NULL);

            g_autoptr(GError) err = NULL;
            TrashItem *trash_item = trash_store_create_trash_item(self, file_info);
            if (err) {
                g_critical("%s:%d: Couldn't create trash item from GFileInfo: %s", __BASE_FILE__, __LINE__, err->message);
                g_object_unref(file_info);
                break;
            }

            gtk_list_box_insert(GTK_LIST_BOX(self->file_box), GTK_WIDGET(trash_item), -1);
            self->trashed_files = g_slist_append(self->trashed_files, trash_item);
            self->file_count++;
            trash_store_check_empty(self);

            g_object_unref(file_info);
            break;
        }
        case G_FILE_MONITOR_EVENT_MOVED_OUT:
        case G_FILE_MONITOR_EVENT_DELETED: {
            g_autofree gchar *file_name = g_file_get_basename(file);
            GSList *elem = g_slist_find_custom(self->trashed_files, file_name, (GCompareFunc) trash_item_has_name);
            TrashItem *trash_item = (TrashItem *) g_slist_nth_data(self->trashed_files,
                                                                   g_slist_position(self->trashed_files, elem));
            g_return_if_fail(trash_item != NULL);

            GtkWidget *row = gtk_widget_get_parent(GTK_WIDGET(trash_item));
            gtk_container_remove(GTK_CONTAINER(self->file_box), row);
            self->file_count--;
            trash_store_check_empty(self);
            self->trashed_files = g_slist_remove(self->trashed_files, trash_item);
            break;
        }
        default:
            break;
    }
}

void trash_store_load_items(TrashStore *self, GError *err) {
    // Open our trash directory
    g_autoptr(GFile) trash_dir = g_file_new_for_path(self->trash_path);
    g_autofree gchar *attributes = g_strconcat(G_FILE_ATTRIBUTE_STANDARD_NAME, ",",
                                               G_FILE_ATTRIBUTE_STANDARD_ICON, ",",
                                               G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                               NULL);
    g_autoptr(GFileEnumerator) enumerator = g_file_enumerate_children(trash_dir,
                                                                      attributes,
                                                                      G_FILE_QUERY_INFO_NONE,
                                                                      NULL,
                                                                      &err);
    if G_UNLIKELY (!enumerator) {
        g_critical("%s:%d: Error getting file enumerator for trash files in '%s': %s", __BASE_FILE__, __LINE__, self->trash_path, err->message);
        return;
    }

    // Iterate over the directory's children and append each file name to a list
    g_autoptr(GFileInfo) current_file = NULL;
    while ((current_file = g_file_enumerator_next_file(enumerator, NULL, &err))) {
        TrashItem *trash_item = trash_store_create_trash_item(self, current_file);

        gtk_list_box_insert(GTK_LIST_BOX(self->file_box), GTK_WIDGET(trash_item), -1);
        self->trashed_files = g_slist_append(self->trashed_files, trash_item);
        self->file_count++;
    }

    trash_store_check_empty(self);
    g_file_enumerator_close(enumerator, NULL, NULL);
}

TrashItem *trash_store_create_trash_item(TrashStore *self, GFileInfo *file_info) {
    gchar *file_name = (gchar *) g_file_info_get_name(file_info);
    g_autofree gchar *info_file_path = g_build_path(G_DIR_SEPARATOR_S, self->trashinfo_path, g_strconcat(file_name, ".trashinfo", NULL), NULL);
    g_autoptr(GFile) info_file = g_file_new_for_path(info_file_path);
    g_autoptr(TrashInfo) trash_info = NULL;

    gboolean has_prefix = (self->path_prefix && !g_str_equal(self->path_prefix, ""));
    if (has_prefix) {
        trash_info = trash_info_new_from_file_with_prefix(info_file, self->path_prefix);
    } else {
        trash_info = trash_info_new_from_file(info_file);
    }

    TrashItem *trash_item = trash_item_new(g_strdup(file_name),
                                           g_build_path(G_DIR_SEPARATOR_S, self->trash_path, file_name, NULL),
                                           g_strdup(info_file_path),
                                           g_file_info_get_icon(file_info),
                                           (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY),
                                           trash_info);
    gtk_widget_show_all(GTK_WIDGET(trash_item));

    return trash_item;
}

gchar *trash_store_read_trash_info(gchar *trashinfo_path, GError **err) {
    // Open the file
    g_autoptr(GFile) info_file = g_file_new_for_path(trashinfo_path);
    g_autoptr(GFileInputStream) input_stream = g_file_read(info_file, NULL, err);
    if (!input_stream) {
        return NULL;
    }

    // Seek to the Path line
    g_seekable_seek(G_SEEKABLE(input_stream), TRASH_INFO_PATH_OFFSET, G_SEEK_SET, NULL, err);

    // Read the file contents and extract the line containing the restore path
    gchar *buffer = (gchar *) malloc(1024 * sizeof(gchar));
    gssize read;
    while ((read = g_input_stream_read(G_INPUT_STREAM(input_stream), buffer, 1024, NULL, err))) {
        buffer[read] = '\0';
    }

    g_input_stream_close(G_INPUT_STREAM(input_stream), NULL, NULL);

    return buffer;
}

gint trash_store_sort(GtkListBoxRow *row1, GtkListBoxRow *row2, __budgie_unused__ gpointer user_data) {
    TrashItem *item1 = TRASH_ITEM(gtk_bin_get_child(GTK_BIN(row1)));
    TrashItem *item2 = TRASH_ITEM(gtk_bin_get_child(GTK_BIN(row2)));

    return trash_item_collate_by_name(item1, item2);
}
