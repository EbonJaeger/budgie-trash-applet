#include "trash_item.h"
#include "applet.h"
#include "utils.h"

enum {
    PROP_EXP_0,
    PROP_FILE_NAME,
    PROP_PATH,
    PROP_TRASHINFO_PATH,
    PROP_RESTORE_PATH,
    PROP_FILE_ICON,
    PROP_IS_DIRECTORY,
    PROP_TIMESTAMP,
    N_EXP_PROPERTIES
};

static GParamSpec *item_props[N_EXP_PROPERTIES] = {
    NULL,
};

struct _TrashItem {
    GtkBox parent_instance;

    gboolean restoring;

    gchar *name;
    gchar *path;
    gchar *trashinfo_path;
    gchar *restore_path;
    GIcon *icon;
    gboolean is_directory;
    gchar *timestamp;

    GtkWidget *header;
    GtkWidget *file_icon;
    GtkWidget *file_name_label;
    GtkWidget *delete_btn;
    GtkWidget *restore_btn;

    GtkWidget *info_revealer;
    GtkWidget *info_container;
    GtkWidget *path_label;
    GtkWidget *timestamp_label;

    TrashRevealer *confirm_revealer;
};

struct _TrashItemClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(TrashItem, trash_item, GTK_TYPE_BOX);

static void trash_item_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec);
static void trash_item_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec);

static void trash_item_class_init(TrashItemClass *klazz) {
    GObjectClass *class = G_OBJECT_CLASS(klazz);
    class->get_property = trash_item_get_property;
    class->set_property = trash_item_set_property;

    item_props[PROP_FILE_NAME] = g_param_spec_string(
        "file-name",
        "File name",
        "Name of the item in the trash bin",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    item_props[PROP_PATH] = g_param_spec_string(
        "path",
        "Path",
        "Path to the item in the trash bin",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    item_props[PROP_TRASHINFO_PATH] = g_param_spec_string(
        "trashinfo-path",
        "Trashinfo path",
        "Path to the .trashinfo file for this item",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    item_props[PROP_RESTORE_PATH] = g_param_spec_string(
        "restore-path",
        "Restore Path",
        "Path to where the trashed item used to be",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    item_props[PROP_FILE_ICON] = g_param_spec_gtype(
        "file-icon",
        "File icon",
        "GIcon to use for this item",
        G_TYPE_NONE,
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    item_props[PROP_IS_DIRECTORY] = g_param_spec_boolean(
        "is-directory",
        "Directory",
        "If this item is a directory or not",
        FALSE,
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    item_props[PROP_TIMESTAMP] = g_param_spec_string(
        "timestamp",
        "Timestamp",
        "The time when the item was deleted",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    g_object_class_install_properties(class, N_EXP_PROPERTIES, item_props);
}

static void trash_item_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    TrashItem *self = TRASH_ITEM(obj);

    switch (prop_id) {
        case PROP_FILE_NAME:
            g_value_set_string(val, self->name);
            break;
        case PROP_PATH:
            g_value_set_string(val, self->path);
            break;
        case PROP_TRASHINFO_PATH:
            g_value_set_string(val, self->trashinfo_path);
            break;
        case PROP_RESTORE_PATH:
            g_value_set_string(val, self->restore_path);
            break;
        case PROP_FILE_ICON:
            g_value_set_gtype(val, (GType) self->icon);
            break;
        case PROP_IS_DIRECTORY:
            g_value_set_boolean(val, self->is_directory);
            break;
        case PROP_TIMESTAMP:
            g_value_set_string(val, self->timestamp);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_item_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    TrashItem *self = TRASH_ITEM(obj);

    switch (prop_id) {
        case PROP_FILE_NAME:
            g_return_if_fail(GTK_IS_WIDGET(self->header));
            trash_item_set_file_name(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_PATH:
            trash_item_set_path(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_TRASHINFO_PATH:
            trash_item_set_trashinfo_path(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_RESTORE_PATH:
            g_return_if_fail(GTK_IS_WIDGET(self->header));
            g_return_if_fail(GTK_IS_WIDGET(self->info_revealer));
            trash_item_set_restore_path(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_FILE_ICON:
            g_return_if_fail(GTK_IS_WIDGET(self->header));
            trash_item_set_icon(self, G_ICON(g_value_get_gtype(val)));
            break;
        case PROP_IS_DIRECTORY:
            trash_item_set_directory(self, g_value_get_boolean(val));
            break;
        case PROP_TIMESTAMP:
            g_return_if_fail(GTK_IS_WIDGET(self->info_revealer));
            trash_item_set_timestamp(self, g_strdup(g_value_get_string(val)));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_item_init(TrashItem *self) {
    self->restoring = FALSE;

    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style, "trash-item");

    // Create the main part of the widget
    self->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    g_object_set(G_OBJECT(self->header), "height-request", 32, NULL);

    // Create the item's delete and restore button
    self->delete_btn = gtk_button_new_from_icon_name("user-trash-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(self->delete_btn, "Delete Item");
    g_signal_connect_object(GTK_BUTTON(self->delete_btn), "clicked", G_CALLBACK(trash_item_handle_btn_clicked), self, 0);

    self->restore_btn = gtk_button_new_from_icon_name("edit-undo-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(self->restore_btn, "Restore Item");
    g_signal_connect_object(GTK_BUTTON(self->restore_btn), "clicked", G_CALLBACK(trash_item_handle_btn_clicked), self, 0);

    self->info_revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(self->info_revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->info_revealer), FALSE);
    GtkStyleContext *revealer_style = gtk_widget_get_style_context(self->info_revealer);
    gtk_style_context_add_class(revealer_style, "trash-info-revealer");

    self->info_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(self->info_revealer), self->info_container);

    self->confirm_revealer = trash_revealer_new();
    g_signal_connect_object(GTK_REVEALER(self->confirm_revealer), "cancel-clicked", G_CALLBACK(trash_item_handle_cancel_clicked), self, 0);
    g_signal_connect_object(GTK_REVEALER(self->confirm_revealer), "confirm-clicked", G_CALLBACK(trash_item_handle_confirm_clicked), self, 0);

    gtk_box_pack_end(GTK_BOX(self->header), self->delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(self->header), self->restore_btn, FALSE, FALSE, 0);

    trash_item_apply_button_styles(self);

    gtk_box_pack_start(GTK_BOX(self), self->header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->info_revealer, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), GTK_WIDGET(self->confirm_revealer), FALSE, FALSE, 0);
    gtk_widget_show_all(GTK_WIDGET(self));
}

TrashItem *trash_item_new(gchar *name,
                          gchar *path,
                          gchar *trashinfo_path,
                          gchar *restore_path,
                          GIcon *icon,
                          gboolean is_directory,
                          gchar *timestamp) {
    return g_object_new(TRASH_TYPE_ITEM,
                        "orientation", GTK_ORIENTATION_VERTICAL,
                        "file-icon", icon,
                        "file-name", name,
                        "path", path,
                        "trashinfo-path", trashinfo_path,
                        "restore-path", restore_path,
                        "is-directory", is_directory,
                        "timestamp", timestamp,
                        NULL);
}

void trash_item_apply_button_styles(TrashItem *self) {
    GtkStyleContext *delete_style = gtk_widget_get_style_context(self->delete_btn);
    gtk_style_context_add_class(delete_style, "flat");
    gtk_style_context_remove_class(delete_style, "button");
    GtkStyleContext *restore_style = gtk_widget_get_style_context(self->restore_btn);
    gtk_style_context_add_class(restore_style, "flat");
    gtk_style_context_remove_class(restore_style, "button");
}

void trash_item_set_btns_sensitive(TrashItem *self, gboolean sensitive) {
    gtk_widget_set_sensitive(self->delete_btn, sensitive);
    gtk_widget_set_sensitive(self->restore_btn, sensitive);
}

void trash_item_set_icon(TrashItem *self, GIcon *icon) {
    if (!GTK_IS_WIDGET(self->header)) {
        return;
    }

    // If we already have an icon set, change it. Else, make a new one and prepend it
    // to our header.
    if (GTK_IS_IMAGE(self->file_icon)) {
        gtk_image_set_from_gicon(GTK_IMAGE(self->file_icon), icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
    } else {
        self->file_icon = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_box_pack_start(GTK_BOX(self->header), self->file_icon, FALSE, FALSE, 5);
    }

    g_object_notify_by_pspec(G_OBJECT(self), item_props[PROP_FILE_ICON]);
}

void trash_item_set_file_name(TrashItem *self, gchar *file_name) {
    gchar *name_clone = g_strdup(file_name);

    if (name_clone == NULL || strcmp(name_clone, "") == 0) {
        return;
    }

    if (!GTK_IS_WIDGET(self->header)) {
        return;
    }

    // Free existing text if it is different
    if ((self->name != NULL) && strcmp(self->name, name_clone) != 0) {
        g_free(self->name);
    }

    self->name = name_clone;

    // If we already have a label, just set new text. Otherwise,
    // create a new label.
    if (GTK_IS_LABEL(self->file_name_label)) {
        gtk_label_set_text(GTK_LABEL(self->file_name_label), self->name);
    } else {
        self->file_name_label = gtk_label_new(self->name);
        gtk_label_set_max_width_chars(GTK_LABEL(self->file_name_label), 30);
        gtk_label_set_ellipsize(GTK_LABEL(self->file_name_label), PANGO_ELLIPSIZE_END);
        gtk_widget_set_halign(self->file_name_label, GTK_ALIGN_START);
        gtk_label_set_justify(GTK_LABEL(self->file_name_label), GTK_JUSTIFY_LEFT);
        gtk_box_pack_end(GTK_BOX(self->header), self->file_name_label, TRUE, TRUE, 0);
    }

    gtk_widget_set_tooltip_text(self->header, self->name);

    g_object_notify_by_pspec(G_OBJECT(self), item_props[PROP_FILE_NAME]);
}

void trash_item_set_path(TrashItem *self, gchar *path) {
    gchar *path_clone = g_strdup(path);

    if (path_clone == NULL || strcmp(path_clone, "") == 0) {
        return;
    }

    // Free existing text if it is different
    if ((self->path != NULL) && strcmp(self->path, path_clone) != 0) {
        g_free(self->path);
    }

    self->path = path_clone;

    g_object_notify_by_pspec(G_OBJECT(self), item_props[PROP_PATH]);
}

void trash_item_set_trashinfo_path(TrashItem *self, gchar *path) {
    gchar *path_clone = g_strdup(path);

    if (path_clone == NULL || strcmp(path_clone, "") == 0) {
        return;
    }

    // Free existing text if it is different
    if ((self->trashinfo_path != NULL) && strcmp(self->trashinfo_path, path_clone) != 0) {
        g_free(self->trashinfo_path);
    }

    self->trashinfo_path = path_clone;

    g_object_notify_by_pspec(G_OBJECT(self), item_props[PROP_TRASHINFO_PATH]);
}

void trash_item_set_restore_path(TrashItem *self, gchar *path) {
    gchar *path_clone = g_strdup(path);

    if (path_clone == NULL || strcmp(path_clone, "") == 0) {
        return;
    }

    if (!GTK_IS_WIDGET(self->header)) {
        return;
    }

    // Free existing text if it is different
    if ((self->restore_path != NULL) && strcmp(self->restore_path, path_clone) != 0) {
        g_free(self->path);
    }

    self->restore_path = path_clone;

    if (GTK_IS_LABEL(self->path_label)) {
        gtk_label_set_markup(GTK_LABEL(self->path_label), g_strconcat("<b>Path:</b> ", self->restore_path, NULL));
    } else {
        self->path_label = gtk_label_new(g_strconcat("<b>Path:</b> ", self->restore_path, NULL));
        gtk_label_set_use_markup(GTK_LABEL(self->path_label), TRUE);
        gtk_label_set_ellipsize(GTK_LABEL(self->path_label), PANGO_ELLIPSIZE_END);
        gtk_widget_set_halign(self->path_label, GTK_ALIGN_START);
        gtk_label_set_justify(GTK_LABEL(self->path_label), GTK_JUSTIFY_LEFT);
        gtk_box_pack_start(GTK_BOX(self->info_container), self->path_label, TRUE, TRUE, 0);
    }

    // Set the tooltip text
    gtk_widget_set_tooltip_text(self->path_label, self->restore_path);

    g_object_notify_by_pspec(G_OBJECT(self), item_props[PROP_RESTORE_PATH]);
}

void trash_item_set_directory(TrashItem *self, gboolean is_directory) {
    self->is_directory = is_directory;
    g_object_notify_by_pspec(G_OBJECT(self), item_props[PROP_IS_DIRECTORY]);
}

void trash_item_set_timestamp(TrashItem *self, gchar *timestamp) {
    gchar *timestamp_clone = g_strdup(timestamp);

    if (timestamp_clone == NULL || strcmp(timestamp_clone, "") == 0) {
        return;
    }

    // Free existing text if it is different
    if ((self->timestamp != NULL) && strcmp(self->timestamp, timestamp_clone) != 0) {
        g_free(self->timestamp);
    }

    self->timestamp = timestamp_clone;

    if (GTK_IS_LABEL(self->timestamp_label)) {
        gtk_label_set_markup(GTK_LABEL(self->timestamp_label), g_strconcat("<b>Deleted at:</b> ", self->timestamp, NULL));
    } else {
        self->timestamp_label = gtk_label_new(g_strconcat("<b>Deleted at:</b> ", self->timestamp, NULL));
        gtk_label_set_use_markup(GTK_LABEL(self->timestamp_label), TRUE);
        gtk_widget_set_halign(self->timestamp_label, GTK_ALIGN_START);
        gtk_label_set_justify(GTK_LABEL(self->timestamp_label), GTK_JUSTIFY_LEFT);
        gtk_box_pack_end(GTK_BOX(self->info_container), self->timestamp_label, TRUE, TRUE, 0);
    }

    g_object_notify_by_pspec(G_OBJECT(self), item_props[PROP_TIMESTAMP]);
}

void trash_item_handle_btn_clicked(GtkButton *sender, TrashItem *self) {
    if (sender == GTK_BUTTON(self->delete_btn)) {
        self->restoring = FALSE;
        trash_revealer_set_text(self->confirm_revealer, "<b>Permanently delete this item?</b>");
    } else {
        self->restoring = TRUE;
        trash_revealer_set_text(self->confirm_revealer, "<b>Restore this item?</b>");
    }

    trash_item_set_btns_sensitive(self, FALSE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->confirm_revealer), TRUE);
}

void trash_item_handle_cancel_clicked(__budgie_unused__ TrashRevealer *sender, TrashItem *self) {
    trash_item_set_btns_sensitive(self, TRUE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->confirm_revealer), FALSE);
}

void trash_item_handle_confirm_clicked(__budgie_unused__ TrashRevealer *sender, TrashItem *self) {
    g_autoptr(GError) err = NULL;
    self->restoring ? trash_item_restore(self, &err) : trash_item_delete(self, &err);
    if (err) {
        g_warning("Error clearing file from trash '%s': %s", self->name, err->message);
    }

    trash_item_set_btns_sensitive(self, TRUE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->confirm_revealer), FALSE);
}

void trash_item_toggle_info_revealer(TrashItem *self) {
    if (gtk_revealer_get_child_revealed(GTK_REVEALER(self->info_revealer))) {
        gtk_revealer_set_reveal_child(GTK_REVEALER(self->info_revealer), FALSE);
    } else {
        gtk_revealer_set_reveal_child(GTK_REVEALER(self->info_revealer), TRUE);
    }
}

void trash_item_delete(TrashItem *self, GError **err) {
    // Delete the trashed file (if it's a directory, it will delete recursively)
    trash_delete_file(self->path, self->is_directory, err);
    g_return_if_fail(err == NULL);

    // Delete the .trashinfo file
    g_autoptr(GFile) info_file = g_file_new_for_path(self->trashinfo_path);
    g_file_delete(info_file, NULL, err);
}

void trash_item_restore(TrashItem *self, GError **err) {
    g_autoptr(GFile) trashed_file = g_file_new_for_path(self->path);
    g_autoptr(GFile) restored_file = g_file_new_for_path(self->restore_path);

    g_file_move(trashed_file, restored_file, G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, err);
    g_return_if_fail(err == NULL);

    // Delete the .trashinfo file
    g_autoptr(GFile) info_file = g_file_new_for_path(self->trashinfo_path);
    g_file_delete(info_file, NULL, err);
}
