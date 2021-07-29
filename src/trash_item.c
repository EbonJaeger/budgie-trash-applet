#include "trash_item.h"

struct _TrashItem {
    GtkBox parent_instance;

    gboolean restoring;

    TrashInfo *trash_info;

    GtkWidget *header;
    GtkWidget *file_icon;
    GtkWidget *file_name_label;
    GtkWidget *delete_btn;
    GtkWidget *restore_btn;

    GtkWidget *info_revealer;
    GtkWidget *info_container;
    GtkWidget *path_label;
    GtkWidget *size_label;
    GtkWidget *timestamp_label;

    TrashRevealer *confirm_revealer;
};

struct _TrashItemClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(TrashItem, trash_item, GTK_TYPE_BOX)

static void trash_item_finalize(GObject *obj);

static void trash_item_class_init(TrashItemClass *klazz) {
    GObjectClass *class = G_OBJECT_CLASS(klazz);
    class->finalize = trash_item_finalize;
}

static void trash_item_init(TrashItem *self) {
    self->restoring = FALSE;

    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style, "trash-item");

    // Create the main part of the widget
    self->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

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
    g_signal_connect_object(GTK_BUTTON(self->confirm_revealer->cancel_button), "clicked", G_CALLBACK(trash_item_handle_cancel_clicked), self, 0);
    g_signal_connect_object(GTK_BUTTON(self->confirm_revealer->confirm_button), "clicked", G_CALLBACK(trash_item_handle_confirm_clicked), self, 0);

    gtk_box_pack_end(GTK_BOX(self->header), self->delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(self->header), self->restore_btn, FALSE, FALSE, 0);

    trash_item_apply_button_styles(self);

    gtk_box_pack_start(GTK_BOX(self), self->header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->info_revealer, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), GTK_WIDGET(self->confirm_revealer), FALSE, FALSE, 0);
}

static void trash_item_finalize(GObject *obj) {
    TrashItem *self = TRASH_ITEM(obj);

    g_slice_free(TrashInfo, self->trash_info);

    G_OBJECT_CLASS(trash_item_parent_class)->finalize(obj);
}

TrashItem *trash_item_new(const gchar *uri) {
    g_autoptr(GFile) file = g_file_new_for_uri(uri);
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInfo) file_info = g_file_query_info(file, TRASH_FILE_ATTRIBUTES, G_FILE_QUERY_INFO_NONE, NULL, &err);

    if (!G_IS_FILE_INFO(file_info)) {
        g_warning("%s:%d: Unable to get file info for '%s': %s", __BASE_FILE__, __LINE__, uri, err->message);
        return NULL;
    }

    gchar *file_name = (gchar *) g_file_info_get_name(file_info);
    TrashInfo *trash_info = NULL;

    // TODO: ew
    trash_info = trash_info_new(file_name,
                                uri,
                                (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY),
                                g_file_info_get_size(file_info));
    trash_info->deleted_time = g_file_info_get_deletion_date(file_info);
    trash_info->restore_path = g_strdup(g_file_info_get_attribute_byte_string(file_info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH));

    // Create the widget
    TrashItem *self = g_object_new(TRASH_TYPE_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
    self->trash_info = trash_info;

    gtk_widget_set_tooltip_text(self->header, self->trash_info->file_name);

    self->file_icon = gtk_image_new_from_gicon(g_file_info_get_icon(file_info), GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_box_pack_start(GTK_BOX(self->header), self->file_icon, FALSE, FALSE, 5);

    self->file_name_label = gtk_label_new(self->trash_info->file_name);
    gtk_label_set_max_width_chars(GTK_LABEL(self->file_name_label), 30);
    gtk_label_set_ellipsize(GTK_LABEL(self->file_name_label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_halign(self->file_name_label, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(self->file_name_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_end(GTK_BOX(self->header), self->file_name_label, TRUE, TRUE, 0);

    self->path_label = gtk_label_new(g_strconcat("<b>Path:</b> ", trash_info->restore_path, NULL));
    gtk_label_set_use_markup(GTK_LABEL(self->path_label), TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(self->path_label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_halign(self->path_label, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(self->path_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start(GTK_BOX(self->info_container), self->path_label, TRUE, TRUE, 0);

    gtk_widget_set_tooltip_text(self->path_label, trash_info->restore_path);

    self->size_label = gtk_label_new(g_strconcat("<b>Size:</b> ", g_format_size(self->trash_info->size), NULL));
    gtk_label_set_use_markup(GTK_LABEL(self->size_label), TRUE);
    gtk_widget_set_halign(self->size_label, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(self->size_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start(GTK_BOX(self->info_container), self->size_label, TRUE, TRUE, 0);

    self->timestamp_label = gtk_label_new(g_strconcat("<b>Deleted at:</b> ", g_date_time_format(trash_info->deleted_time, "%d %b %Y %X"), NULL));
    gtk_label_set_use_markup(GTK_LABEL(self->timestamp_label), TRUE);
    gtk_widget_set_halign(self->timestamp_label, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(self->timestamp_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_end(GTK_BOX(self->info_container), self->timestamp_label, TRUE, TRUE, 0);

    gtk_widget_show_all(GTK_WIDGET(self));

    return self;
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

gint trash_item_has_name(TrashItem *self, gchar *name) {
    return g_strcmp0(self->trash_info->file_name, name);
}

void trash_item_handle_btn_clicked(GtkButton *sender, TrashItem *self) {
    if (sender == GTK_BUTTON(self->delete_btn)) {
        self->restoring = FALSE;
        trash_revealer_set_text(self->confirm_revealer, "<b>Permanently delete this item?</b>", TRUE);
    } else {
        self->restoring = TRUE;
        trash_revealer_set_text(self->confirm_revealer, "<b>Restore this item?</b>", FALSE);
    }

    trash_item_set_btns_sensitive(self, FALSE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->confirm_revealer), TRUE);
}

void trash_item_handle_cancel_clicked(__attribute__((unused)) GtkButton *sender, TrashItem *self) {
    trash_item_set_btns_sensitive(self, TRUE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->confirm_revealer), FALSE);
}

void trash_item_handle_confirm_clicked(__attribute__((unused)) GtkButton *sender, TrashItem *self) {
    g_autoptr(GError) err = NULL;
    self->restoring ? trash_item_restore(self, err) : trash_item_delete(self, err);

    if (err) {
        g_autofree gchar *body = g_strconcat("Error ", self->restoring ? "restoring" : "deleting", " '", self->trash_info->file_name, "' from trash bin: ", err->message, NULL);
        trash_notify_try_send("Trash Bin Error", body, "dialog-error-symbolic");
    }
}

void trash_item_toggle_info_revealer(TrashItem *self) {
    if (gtk_revealer_get_child_revealed(GTK_REVEALER(self->info_revealer))) {
        gtk_revealer_set_reveal_child(GTK_REVEALER(self->info_revealer), FALSE);
    } else {
        gtk_revealer_set_reveal_child(GTK_REVEALER(self->info_revealer), TRUE);
    }
}

void trash_item_delete(TrashItem *self, GError *err) {
    GFile *file = g_file_new_for_uri(self->trash_info->file_path);
    if (!G_IS_FILE(file)) {
        return;
    }

    g_file_delete_async(file, G_PRIORITY_DEFAULT, NULL, (GAsyncReadyCallback) trash_item_delete_finish, err);
}

void trash_item_delete_finish(GFile *file, GAsyncResult *result, GError *err) {
    g_file_delete_finish(file, result, &err);
}

void trash_item_restore(TrashItem *self, GError *err) {
    g_autoptr(GFile) trashed_file = g_file_new_for_uri(self->trash_info->file_path);
    g_autoptr(GFile) restored_file = g_file_new_for_path(self->trash_info->restore_path);

    if (!G_IS_FILE(trashed_file)) {
        return;
    }

    if (!G_IS_FILE(restored_file)) {
        return;
    }

    g_file_move(trashed_file, restored_file, G_FILE_COPY_ALL_METADATA, NULL, NULL, NULL, &err);
}

gint trash_item_collate_by_date(TrashItem *self, TrashItem *other) {
    return g_date_time_compare(self->trash_info->deleted_time, other->trash_info->deleted_time);
}

gint trash_item_collate_by_name(TrashItem *self, TrashItem *other) {
    return strcoll(self->trash_info->file_name, other->trash_info->file_name);
}

gint trash_item_collate_by_type(TrashItem *self, TrashItem *other) {
    gint ret = 0;

    if (self->trash_info->is_directory && other->trash_info->is_directory) {
        ret = strcoll(self->trash_info->file_name, other->trash_info->file_name);
    } else if (self->trash_info->is_directory && !other->trash_info->is_directory) {
        ret = -1;
    } else if (!self->trash_info->is_directory && other->trash_info->is_directory) {
        ret = 1;
    } else {
        ret = strcoll(self->trash_info->file_name, other->trash_info->file_name);
    }

    return ret;
}
