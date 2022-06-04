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

    TrashConfirmDialog *confirm_revealer;
};

G_DEFINE_TYPE (TrashItem, trash_item, GTK_TYPE_BOX)

static void trash_item_finalize (GObject *obj);

static void trash_item_class_init (TrashItemClass *klazz) {
    GObjectClass *class = G_OBJECT_CLASS (klazz);
    class->finalize = trash_item_finalize;
}

static void set_buttons_sensitive (TrashItem *self, gboolean sensitive) {
    gtk_widget_set_sensitive (self->delete_btn, sensitive);
    gtk_widget_set_sensitive (self->restore_btn, sensitive);
}

static void response_ok (TrashItem *self) {
    g_autofree gchar *notif_body = NULL;
    g_autoptr (GError) err = NULL;
    gboolean success;

    if (self->restoring) {
        trash_item_restore (self, err);
    } else {
        trash_item_delete (self, err);
    }

    success = err == NULL;

    if (!success) {
        notif_body = g_strdup_printf (
            "Error %s '%s' from trash bin: %s",
            self->restoring ? "restoring" : "deleting",
            trash_info_get_name (self->trash_info),
            err->message
        );
        trash_notify_try_send ("Trash Bin Error", notif_body, "dialog-error-symbolic");
    }
}

static void dialog_response_cb (TrashConfirmDialog *dialog, gint response_id, TrashItem *self) {
    switch (response_id)
    {
    case GTK_RESPONSE_CANCEL:
        set_buttons_sensitive (self, TRUE);
        gtk_revealer_set_reveal_child (GTK_REVEALER (dialog), FALSE);
        break;

    case GTK_RESPONSE_OK:
        response_ok (self);
        break;
    
    default:
        break;
    }
}

static void button_clicked(GtkButton *sender, TrashItem *self) {
    if (sender == GTK_BUTTON(self->delete_btn)) {
        self->restoring = FALSE;
        trash_confirm_dialog_show_message(self->confirm_revealer, "<b>Permanently delete this item?</b>", TRUE);
    } else {
        self->restoring = TRUE;
        trash_confirm_dialog_show_message(self->confirm_revealer, "<b>Restore this item?</b>", FALSE);
    }

    set_buttons_sensitive(self, FALSE);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->confirm_revealer), TRUE);
}

static void apply_button_styles(TrashItem *self) {
    GtkStyleContext *delete_style = gtk_widget_get_style_context(self->delete_btn);
    gtk_style_context_add_class(delete_style, "flat");
    gtk_style_context_remove_class(delete_style, "button");
    GtkStyleContext *restore_style = gtk_widget_get_style_context(self->restore_btn);
    gtk_style_context_add_class(restore_style, "flat");
    gtk_style_context_remove_class(restore_style, "button");
}

static void trash_item_init (TrashItem *self) {
    self->restoring = FALSE;

    GtkStyleContext *style = gtk_widget_get_style_context (GTK_WIDGET (self));
    gtk_style_context_add_class (style, "trash-item");

    // Create the main part of the widget
    self->header = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    // Create the item's delete and restore button
    self->delete_btn = gtk_button_new_from_icon_name ("user-trash-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text (self->delete_btn, "Delete Item");

    self->restore_btn = gtk_button_new_from_icon_name ("edit-undo-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text (self->restore_btn, "Restore Item");

    self->info_revealer = gtk_revealer_new ();
    gtk_revealer_set_transition_type (GTK_REVEALER (self->info_revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child (GTK_REVEALER (self->info_revealer), FALSE);
    GtkStyleContext *revealer_style = gtk_widget_get_style_context (self->info_revealer);
    gtk_style_context_add_class (revealer_style, "trash-info-dialog");

    self->info_container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add (GTK_CONTAINER (self->info_revealer), self->info_container);

    self->confirm_revealer = trash_confirm_dialog_new ();

    apply_button_styles (self);

    /* Signals */

    g_signal_connect (
        GTK_BUTTON (self->delete_btn),
        "clicked",
        G_CALLBACK (button_clicked),
        self
    );

    g_signal_connect (
        GTK_BUTTON (self->restore_btn),
        "clicked",
        G_CALLBACK (button_clicked),
        self
    );

    g_signal_connect (
        self->confirm_revealer,
        "response",
        G_CALLBACK (dialog_response_cb),
        self
    );

    /* Packing */

    gtk_box_pack_end (GTK_BOX (self->header), self->delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (self->header), self->restore_btn, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (self), self->header, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (self), self->info_revealer, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (self->confirm_revealer), FALSE, FALSE, 0);
}

static void trash_item_finalize (GObject *obj) {
    g_return_if_fail (obj != NULL);
    g_return_if_fail (TRASH_IS_ITEM (obj));

    TrashItem *self = TRASH_ITEM (obj);

    g_return_if_fail (self != NULL);

    g_object_unref (self->trash_info);

    G_OBJECT_CLASS (trash_item_parent_class)->finalize (obj);
}

TrashItem *trash_item_new (TrashInfo *trash_info) {
    if (!TRASH_IS_INFO (trash_info)) {
        return NULL;
    }

    // Create the widget
    TrashItem *self = g_object_new (TRASH_TYPE_ITEM, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
    self->trash_info = trash_info;

    gtk_widget_set_tooltip_text (self->header, trash_info_get_name (trash_info));

    self->file_icon = gtk_image_new_from_gicon (trash_info_get_icon (trash_info), GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_box_pack_start (GTK_BOX (self->header), self->file_icon, FALSE, FALSE, 5);

    self->file_name_label = gtk_label_new (trash_info_get_name (trash_info));
    gtk_label_set_max_width_chars (GTK_LABEL (self->file_name_label), 30);
    gtk_label_set_ellipsize (GTK_LABEL (self->file_name_label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_halign (self->file_name_label, GTK_ALIGN_START);
    gtk_label_set_justify (GTK_LABEL (self->file_name_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_end (GTK_BOX (self->header), self->file_name_label, TRUE, TRUE, 0);

    self->path_label = gtk_label_new (g_strdup_printf ("<b>Path:</b> %s", trash_info_get_restore_path (trash_info)));
    gtk_label_set_use_markup (GTK_LABEL (self->path_label), TRUE);
    gtk_label_set_ellipsize (GTK_LABEL (self->path_label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_halign (self->path_label, GTK_ALIGN_START);
    gtk_label_set_justify (GTK_LABEL (self->path_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (self->info_container), self->path_label, TRUE, TRUE, 0);

    gtk_widget_set_tooltip_text (self->path_label, trash_info_get_restore_path (trash_info));

    self->size_label = gtk_label_new (g_strdup_printf ("<b>Size:</b> %s", g_format_size (trash_info_get_size (trash_info))));
    gtk_label_set_use_markup (GTK_LABEL (self->size_label), TRUE);
    gtk_widget_set_halign (self->size_label, GTK_ALIGN_START);
    gtk_label_set_justify (GTK_LABEL (self->size_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (self->info_container), self->size_label, TRUE, TRUE, 0);

    self->timestamp_label = gtk_label_new (g_strdup_printf ("<b>Deleted at:</b> %s",
                                                            g_date_time_format (
                                                                trash_info_get_deletion_time (trash_info),
                                                                "%d %b %Y %X")));
    gtk_label_set_use_markup (GTK_LABEL (self->timestamp_label), TRUE);
    gtk_widget_set_halign (self->timestamp_label, GTK_ALIGN_START);
    gtk_label_set_justify (GTK_LABEL (self->timestamp_label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_end (GTK_BOX (self->info_container), self->timestamp_label, TRUE, TRUE, 0);

    gtk_widget_show_all (GTK_WIDGET (self));

    return self;
}

gint trash_item_has_name (TrashItem *self, gchar *name) {
    return g_strcmp0 (trash_info_get_name (self->trash_info), name);
}

void trash_item_toggle_info_revealer (TrashItem *self) {
    if (gtk_revealer_get_child_revealed (GTK_REVEALER (self->info_revealer))) {
        gtk_revealer_set_reveal_child (GTK_REVEALER (self->info_revealer), FALSE);
    } else {
        gtk_revealer_set_reveal_child (GTK_REVEALER (self->info_revealer), TRUE);
    }
}

static void delete_finish (GFile *file, GAsyncResult *result, GError *err) {
    g_file_delete_finish (file, result, &err);
}

void trash_item_delete (TrashItem *self, GError *err) {
    GFile *file = g_file_new_for_uri (trash_info_get_uri (self->trash_info));
    
    g_return_if_fail (G_IS_FILE (file));

    g_file_delete_async (
        file,
        G_PRIORITY_DEFAULT,
        NULL,
        (GAsyncReadyCallback) delete_finish,
        err
    );
}

static void restore_finish (GFile *file, GAsyncResult *result, GError *err) {
    g_file_move_finish (file, result, &err);
}

void trash_item_restore (TrashItem *self, GError *err) {
    g_autoptr(GFile) trashed_file = g_file_new_for_uri (trash_info_get_uri (self->trash_info));
    g_autoptr(GFile) restored_file = g_file_new_for_path (trash_info_get_restore_path (self->trash_info));

    g_return_if_fail (G_IS_FILE (trashed_file));
    g_return_if_fail (G_IS_FILE (restored_file));

    g_file_move_async (
        trashed_file,
        restored_file,
        G_FILE_COPY_ALL_METADATA,
        G_PRIORITY_DEFAULT,
        NULL, NULL, NULL,
        (GAsyncReadyCallback) restore_finish,
        err
    );
}

gint trash_item_collate_by_date (TrashItem *self, TrashItem *other) {
    return g_date_time_compare (
        trash_info_get_deletion_time (self->trash_info),
        trash_info_get_deletion_time (other->trash_info)
    );
}

gint trash_item_collate_by_name (TrashItem *self, TrashItem *other) {
    return strcoll (
        trash_info_get_name (self->trash_info),
        trash_info_get_name (other->trash_info)
    );
}

gint trash_item_collate_by_type (TrashItem *self, TrashItem *other) {
    gint ret = 0;

    if (trash_info_is_directory (self->trash_info) && trash_info_is_directory (other->trash_info)) {
        ret = trash_item_collate_by_name (self, other);
    } else if (trash_info_is_directory (self->trash_info) && !trash_info_is_directory (other->trash_info)) {
        ret = -1;
    } else if (!trash_info_is_directory (self->trash_info) && trash_info_is_directory (other->trash_info)) {
        ret = 1;
    } else {
        ret = trash_item_collate_by_name (self, other);
    }

    return ret;
}
