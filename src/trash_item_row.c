#include "trash_item_row.h"

enum {
    PROPERTY_0,
    TRASH_INFO_PROPERTY,
    N_PROPERTIES
};

static GParamSpec *props[N_PROPERTIES];

struct _TrashItemRow {
    GtkListBoxRow parent_instance;

    TrashInfo *trash_info;

    GtkWidget *header;
    GtkWidget *delete_btn;
    TrashConfirmDialog *confirm_revealer;
};

G_DEFINE_FINAL_TYPE(TrashItemRow, trash_item_row, GTK_TYPE_LIST_BOX_ROW)

static void delete_clicked_cb(GtkButton *source, gpointer user_data) {
    (void) source;
    
    TrashItemRow *self = user_data;
    
    gtk_revealer_set_reveal_child(GTK_REVEALER(self->confirm_revealer), TRUE);
}

static void confirm_response_cb(TrashConfirmDialog *source, GtkResponseType type, gpointer user_data) {
    TrashItemRow *self = user_data;
    
    gtk_revealer_set_reveal_child(GTK_REVEALER(source), FALSE);
    
    switch (type) {
        case GTK_RESPONSE_OK:
            trash_item_row_delete(self);
            break;
        default:
            break;
    }
}

static void trash_item_row_constructed(GObject *object) {
    TrashItemRow *self;

    GVariant *raw_icon;
    GIcon *gicon;
    const gchar *name;
    const gchar *path;
    GDateTime *deletion_time;
    gchar *formatted_date;

    GtkWidget *grid;
    GtkWidget *icon;
    GtkWidget *name_label;
    GtkWidget *date_label;
    GtkStyleContext *date_style_context;
    PangoAttrList *attr_list;
    PangoFontDescription *font_description;
    PangoAttribute *font_attr;

    GtkStyleContext *delete_button_style;

    self = TRASH_ITEM_ROW(object);

    g_object_get(
        self->trash_info,
        "display-name", &name,
        "icon", &raw_icon,
        "restore-path", &path,
        "deletion-time", &deletion_time,
        NULL
    );

    gicon = g_icon_deserialize(raw_icon);
    icon = gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_widget_set_margin_start(icon, 6);
    gtk_widget_set_margin_end(icon, 6);

    name_label = gtk_label_new(name);
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    gtk_widget_set_valign(name_label, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(name_label, TRUE);
    gtk_widget_set_tooltip_text(name_label, path);

    attr_list = pango_attr_list_new();
    font_description = pango_font_description_new();
    pango_font_description_set_stretch(font_description, PANGO_STRETCH_ULTRA_CONDENSED);
    pango_font_description_set_weight(font_description, PANGO_WEIGHT_SEMILIGHT);
    font_attr = pango_attr_font_desc_new(font_description);
    pango_attr_list_insert(attr_list, font_attr);

    formatted_date = g_date_time_format(deletion_time, "%d %b %Y %X");
    date_label = gtk_label_new(formatted_date);
    gtk_widget_set_halign(date_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(date_label, TRUE);
    gtk_label_set_attributes(GTK_LABEL(date_label), attr_list);
    date_style_context = gtk_widget_get_style_context(date_label);
    gtk_style_context_add_class(date_style_context, GTK_STYLE_CLASS_DIM_LABEL);

    self->delete_btn = gtk_button_new_from_icon_name("user-trash-symbolic", GTK_ICON_SIZE_BUTTON);
    delete_button_style = gtk_widget_get_style_context(self->delete_btn);
    gtk_style_context_add_class(delete_button_style, GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
    gtk_style_context_add_class(delete_button_style, GTK_STYLE_CLASS_FLAT);
    gtk_style_context_add_class(delete_button_style, "circular");
    gtk_widget_set_tooltip_text(self->delete_btn, "Permanently this item");
    
    self->confirm_revealer = trash_confirm_dialog_new("Are you sure you want to delete this item?", TRUE);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_widget_set_margin_top(GTK_WIDGET(self), 2);
    gtk_widget_set_margin_bottom(GTK_WIDGET(self), 2);

    gtk_grid_attach(GTK_GRID(grid), icon, 0, 0, 2, 2);
    gtk_grid_attach(GTK_GRID(grid), name_label, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), self->delete_btn, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), date_label, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(self->confirm_revealer), 0, 3, 4, 1);

    gtk_container_add(GTK_CONTAINER(self), grid);

    gtk_widget_set_margin_end(GTK_WIDGET(self), 10);
    gtk_widget_show_all(GTK_WIDGET(self));
    
    g_signal_connect(self->delete_btn, "clicked", G_CALLBACK(delete_clicked_cb), self);
    g_signal_connect(self->confirm_revealer, "response", G_CALLBACK(confirm_response_cb), self);

    G_OBJECT_CLASS(trash_item_row_parent_class)->constructed(object);
}

static void trash_item_row_finalize(GObject *object) {
    TrashItemRow *self;

    self = TRASH_ITEM_ROW(object);

    g_object_unref(self->trash_info);

    G_OBJECT_CLASS(trash_item_row_parent_class)->finalize(object);
}

static void trash_item_row_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *spec) {
    TrashItemRow *self;

    self = TRASH_ITEM_ROW(object);

    switch (prop_id) {
        case TRASH_INFO_PROPERTY:
            g_value_set_pointer(value, trash_item_row_get_info(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, spec);
            break;
    }
}

static void trash_item_row_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *spec) {
    TrashItemRow *self;
    gpointer pointer;

    self = TRASH_ITEM_ROW(object);

    switch (prop_id) {
        case TRASH_INFO_PROPERTY:
            pointer = g_value_get_pointer(value);
            self->trash_info = g_object_ref_sink(pointer);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, spec);
            break;
    }
}

static void trash_item_row_class_init(TrashItemRowClass *klazz) {
    GObjectClass *class;

    class = G_OBJECT_CLASS(klazz);
    class->constructed = trash_item_row_constructed;
    class->finalize = trash_item_row_finalize;
    class->get_property = trash_item_row_get_property;
    class->set_property = trash_item_row_set_property;

    // Properties

    props[TRASH_INFO_PROPERTY] = g_param_spec_pointer(
        "trash-info",
        "Trash info",
        "The information for this row",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    g_object_class_install_properties(class, N_PROPERTIES, props);
}

static void trash_item_row_init(TrashItemRow *self) {
    (void) self;
}

TrashItemRow *trash_item_row_new(TrashInfo *trash_info) {
    return g_object_new(TRASH_TYPE_ITEM_ROW, "trash-info", trash_info, NULL);
}

TrashInfo *trash_item_row_get_info(TrashItemRow *self) {
    return g_object_ref(self->trash_info);
}

static void delete_finish(GObject *object, GAsyncResult *result, gpointer user_data) {
    (void) user_data;

    GFile *file;
    g_autoptr(GError) error = NULL;

    file = G_FILE(object);

    g_file_delete_finish(file, result, &error);

    if (error) {
        g_critical("Error deleting file '%s': %s", g_file_get_basename(file), error->message);
    }
}

/**
 * Asynchronously deletes a trashed item.
 */
void trash_item_row_delete(TrashItemRow *self) {
    g_autoptr(GFile) file;
    g_autofree const gchar *name;
    g_autofree gchar *uri;

    name = trash_info_get_name(self->trash_info);
    uri = g_strdup_printf("trash:///%s", name);
    file = g_file_new_for_uri(uri);

    g_file_delete_async(
        file,
        G_PRIORITY_DEFAULT,
        NULL,
        delete_finish,
        NULL
    );
}

void restore_finish(GObject *object, GAsyncResult *result, gpointer user_data) {
    (void) user_data;

    gboolean success;
    g_autoptr(GError) error = NULL;

    success = g_file_move_finish(G_FILE(object), result, &error);

    if (!success) {
        g_critical("Error restoring file '%s' to '%s': %s", g_file_get_basename(G_FILE(object)), g_file_get_path(G_FILE(object)), error->message);
    }
}

/**
 * Asynchronously restores a trashed item to its original location.
 */
void trash_item_row_restore(TrashItemRow *self) {
    g_autoptr(GFile) file, restored_file;
    g_autofree const gchar *name;
    g_autofree gchar *uri;
    g_autofree const gchar *restore_path;

    name = trash_info_get_name(self->trash_info);
    uri = g_strdup_printf("trash:///%s", name);
    file = g_file_new_for_uri(uri);
    restore_path = trash_info_get_restore_path(self->trash_info);
    restored_file = g_file_new_for_path(restore_path);

    g_file_move_async(
        file,
        restored_file,
        G_FILE_COPY_ALL_METADATA,
        G_PRIORITY_DEFAULT,
        NULL, NULL, NULL,
        restore_finish,
        NULL);
}

/**
 * Compares two TrashItems for sorting, putting them in order by deletion date
 * in ascending order.
 */
gint trash_item_row_collate_by_date(TrashItemRow *self, TrashItemRow *other) {
    return g_date_time_compare(
        trash_info_get_deletion_time(self->trash_info),
        trash_info_get_deletion_time(other->trash_info)
    );
}

/**
 * Compares two TrashItems for sorting, putting them in alphabetical order.
 */
gint trash_item_row_collate_by_name(TrashItemRow *self, TrashItemRow *other) {
    return strcoll(
        trash_info_get_name(self->trash_info),
        trash_info_get_name(other->trash_info)
    );
}

/**
 * Compares two TrashItems for sorting. This function uses the following rules:
 *
 * 1. Directories should be above regular files
 * 2. Directories should be sorted alphabetically
 * 3. Files should be sorted alphabetically
 */
gint trash_item_row_collate_by_type(TrashItemRow *self, TrashItemRow *other) {
    gint ret = 0;

    if (trash_info_is_directory(self->trash_info) && trash_info_is_directory(other->trash_info)) {
        ret = trash_item_row_collate_by_name(self, other);
    } else if (trash_info_is_directory(self->trash_info) && !trash_info_is_directory(other->trash_info)) {
        ret = -1;
    } else if (!trash_info_is_directory(self->trash_info) && trash_info_is_directory(other->trash_info)) {
        ret = 1;
    } else {
        ret = trash_item_row_collate_by_name(self, other);
    }

    return ret;
}
