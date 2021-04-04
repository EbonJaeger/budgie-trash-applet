#include "trash_store.h"

enum
{
    PROP_EXP_0,
    PROP_DRIVE_NAME,
    PROP_ICON_NAME,
    N_EXP_PROPERTIES
};

static GParamSpec *store_props[N_EXP_PROPERTIES] = {
    NULL,
};

struct _TrashStore
{
    GtkBox parent_instance;

    gchar *drive_name;
    gchar *icon_name;

    GtkWidget *header;
    GtkWidget *header_icon;
    GtkWidget *header_label;
    GtkWidget *file_box;
};

struct _TrashStoreClass
{
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(TrashStore, trash_store, GTK_TYPE_BOX);

static void trash_store_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec);
static void trash_store_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec);

static void trash_store_class_init(TrashStoreClass *klazz)
{
    GObjectClass *class = G_OBJECT_CLASS(klazz);
    class->get_property = trash_store_get_property;
    class->set_property = trash_store_set_property;

    store_props[PROP_DRIVE_NAME] = g_param_spec_string(
        "drive-name",
        "Drive Name",
        "Name of the drive where the trash bin is located",
        "This PC",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    store_props[PROP_ICON_NAME] = g_param_spec_string(
        "icon-name",
        "Icon Name",
        "Name of the icon to use for this drive",
        "drive-harddisk-symbolic",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    g_object_class_install_properties(class, N_EXP_PROPERTIES, store_props);
}

static void trash_store_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec)
{
    TrashStore *self = TRASH_STORE(obj);

    switch (prop_id)
    {
    case PROP_DRIVE_NAME:
        g_value_set_string(val, self->drive_name);
        break;
    case PROP_ICON_NAME:
        g_value_set_string(val, self->icon_name);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
        break;
    }
}

static void trash_store_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec)
{
    TrashStore *self = TRASH_STORE(obj);

    switch (prop_id)
    {
    case PROP_DRIVE_NAME:
        g_return_if_fail(GTK_IS_WIDGET(self->header));
        trash_store_set_drive_name(self, g_strdup(g_value_get_string(val)));
        break;
    case PROP_ICON_NAME:
        g_return_if_fail(GTK_IS_WIDGET(self->header));
        trash_store_set_icon_name(self, g_strdup(g_value_get_string(val)));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
        break;
    }
}

static void trash_store_init(TrashStore *self)
{
    GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style, "trash-store-widget");
    gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);

    // Create our header box
    self->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkStyleContext *header_style = gtk_widget_get_style_context(self->header);
    gtk_style_context_add_class(header_style, "trash-store-widget");
    g_object_set(G_OBJECT(self->header), "height-request", 32, NULL);

    // Create our file list
    GtkWidget *file_box = gtk_list_box_new();
    GtkStyleContext *file_box_style = gtk_widget_get_style_context(file_box);
    gtk_style_context_add_class(file_box_style, "trash-file-box");
    gtk_style_context_add_class(file_box_style, "empty");
    gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(file_box), TRUE);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(file_box), GTK_SELECTION_NONE);
    self->file_box = file_box;

    // Pack ourselves up
    gtk_box_pack_start(GTK_BOX(self), self->header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(self), file_box, TRUE, TRUE, 0);
}

TrashStore *trash_store_new(gchar *drive_name, gchar *icon_name)
{
    return g_object_new(TRASH_TYPE_STORE,
                        "orientation", GTK_ORIENTATION_VERTICAL,
                        "drive-name", drive_name,
                        "icon-name", icon_name,
                        NULL);
}

void trash_store_set_drive_name(TrashStore *self, gchar *drive_name)
{
    if (drive_name == NULL || strcmp(drive_name, "") == 0)
    {
        return;
    }

    if (!GTK_IS_WIDGET(self->header))
    {
        return;
    }

    // Free existing text if it is different
    if ((self->drive_name != NULL) && strcmp(self->drive_name, drive_name) != 0)
    {
        g_free(self->drive_name);
    }

    self->drive_name = drive_name;

    // If we already have a label, just set new text. Otherwise,
    // Create a new label.
    if (GTK_IS_LABEL(self->header_label))
    {
        gtk_label_set_text(GTK_LABEL(self->header_label), self->drive_name);
    }
    else
    {
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

void trash_store_set_icon_name(TrashStore *self, gchar *icon_name)
{
    if (icon_name == NULL || strcmp(icon_name, "") == 0)
    {
        return;
    }

    if (!GTK_IS_WIDGET(self->header))
    {
        return;
    }

    // Free existing text if it is different
    if ((self->icon_name != NULL) && strcmp(self->icon_name, icon_name) != 0)
    {
        g_free(self->icon_name);
    }

    self->icon_name = icon_name;

    // If we already have an icon set, change it. Else, make a new one and prepend it
    // to our header.
    if (GTK_IS_IMAGE(self->header_icon))
    {
        gtk_image_set_from_icon_name(GTK_IMAGE(self->header_icon), self->icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
    }
    else
    {
        self->header_icon = gtk_image_new_from_icon_name(self->icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_box_pack_start(GTK_BOX(self->header), self->header_icon, FALSE, FALSE, 10);
    }

    g_object_notify_by_pspec(G_OBJECT(self), store_props[PROP_ICON_NAME]);
}
