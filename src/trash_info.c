#include "trash_info.h"

enum {
    PROP_0,
    PROP_NAME,
    PROP_DISPLAY_NAME,
    PROP_URI,
    PROP_RESTORE_PATH,
    PROP_ICON,
    PROP_SIZE,
    PROP_IS_DIR,
    PROP_DELETION_TIME,
    N_PROPS
};

static GParamSpec *props[N_PROPS];

struct _TrashInfo {
    GObject parent_instance;

    const gchar *name;
    const gchar *display_name;
    const gchar *uri;
    const gchar *restore_path;

    GIcon *icon;

    goffset size;
    gboolean is_directory;

    GDateTime *deleted_time;
};

G_DEFINE_FINAL_TYPE(TrashInfo, trash_info, G_TYPE_OBJECT);

static void trash_info_finalize(GObject *obj) {
    TrashInfo *self;

    self = TRASH_INFO(obj);

    g_free((gchar *) self->name);
    g_free((gchar *) self->display_name);
    g_free((gchar *) self->uri);
    g_free((gchar *) self->restore_path);
    g_object_unref(self->icon);
    g_date_time_unref(self->deleted_time);

    G_OBJECT_CLASS(trash_info_parent_class)->finalize(obj);
}

static void trash_info_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *spec) {
    TrashInfo *self;
    GIcon *icon;

    self = TRASH_INFO(obj);

    switch (prop_id) {
        case PROP_NAME:
            g_value_set_string(value, trash_info_get_name(self));
            break;
        case PROP_DISPLAY_NAME:
            g_value_set_string(value, trash_info_get_display_name(self));
            break;
        case PROP_URI:
            g_value_set_string(value, trash_info_get_uri(self));
            break;
        case PROP_RESTORE_PATH:
            g_value_set_string(value, trash_info_get_restore_path(self));
            break;
        case PROP_ICON:
            icon = trash_info_get_icon(self);
            g_value_set_variant(value, g_icon_serialize(icon));
            break;
        case PROP_SIZE:
            g_value_set_uint64(value, self->size);
            break;
        case PROP_IS_DIR:
            g_value_set_boolean(value, self->is_directory);
            break;
        case PROP_DELETION_TIME:
            g_value_set_pointer(value, trash_info_get_deletion_time(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_info_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *spec) {
    TrashInfo *self;
    GVariant *raw_icon;
    gpointer date_pointer;

    self = TRASH_INFO(obj);

    switch (prop_id) {
        case PROP_NAME:
            self->name = g_value_get_string(value);
            break;
        case PROP_DISPLAY_NAME:
            self->display_name = g_value_get_string(value);
            break;
        case PROP_URI:
            self->uri = g_value_get_string(value);
            break;
        case PROP_RESTORE_PATH:
            self->restore_path = g_value_get_string(value);
            break;
        case PROP_ICON:
            raw_icon = g_value_get_variant(value);
            self->icon = g_icon_deserialize(raw_icon);
            break;
        case PROP_SIZE:
            self->size = g_value_get_uint64(value);
            break;
        case PROP_IS_DIR:
            self->is_directory = g_value_get_boolean(value);
            break;
        case PROP_DELETION_TIME:
            date_pointer = g_value_get_pointer(value);
            self->deleted_time = (GDateTime *) date_pointer;
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_info_class_init(TrashInfoClass *klazz) {
    GObjectClass *class = G_OBJECT_CLASS(klazz);
    class->finalize = trash_info_finalize;
    class->get_property = trash_info_get_property;
    class->set_property = trash_info_set_property;

    props[PROP_NAME] = g_param_spec_string(
        "name",
        "file name",
        "The name of the file",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_DISPLAY_NAME] = g_param_spec_string(
        "display-name",
        "Display name",
        "The display name of the file",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_URI] = g_param_spec_string(
        "uri",
        "URI",
        "The URI to the file",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_RESTORE_PATH] = g_param_spec_string(
        "restore-path",
        "restore path",
        "The original path to the file",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_ICON] = g_param_spec_variant(
        "icon",
        "file icon",
        "The display icon for the file",
        G_VARIANT_TYPE_ANY,
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_SIZE] = g_param_spec_uint64(
        "size",
        "file size",
        "The size of the file on disk",
        0,
        G_MAXINT64,
        0,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_IS_DIR] = g_param_spec_boolean(
        "is-dir",
        "is directory",
        "If the file is a directory or not",
        FALSE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_DELETION_TIME] = g_param_spec_pointer(
        "deletion-time",
        "deletion time",
        "The timestamp of when the file was deleted",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    g_object_class_install_properties(class, N_PROPS, props);
}

static void trash_info_init(__attribute__((unused)) TrashInfo *self) {}

TrashInfo *trash_info_new(GFileInfo *info, const gchar *uri) {
    GIcon *icon;

    icon = g_file_info_get_icon(info);

    return g_object_new(
        TRASH_TYPE_INFO,
        "name", g_strdup(g_file_info_get_name(info)),
        "display-name", g_strdup(g_file_info_get_display_name(info)),
        "uri", g_strdup(uri),
        "restore-path", g_strdup(g_file_info_get_attribute_byte_string(info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH)),
        "icon", g_icon_serialize(g_object_ref(icon)),
        "size", g_file_info_get_size(info),
        "is-dir", (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY),
        "deletion-time", g_date_time_ref(g_file_info_get_deletion_date(info)),
        NULL
    );
}

/* Property getters */

const gchar *trash_info_get_name(TrashInfo *self) {
    return g_strdup(self->name);
}

const gchar *trash_info_get_display_name(TrashInfo *self) {
    return g_strdup(self->display_name);
}

const gchar *trash_info_get_uri(TrashInfo *self) {
    return g_strdup(self->uri);
}

const gchar *trash_info_get_restore_path(TrashInfo *self) {
    return g_strdup(self->restore_path);
}

GIcon *trash_info_get_icon(TrashInfo *self) {
    return g_object_ref(self->icon);
}

goffset trash_info_get_size(TrashInfo *self) {
    return self->size;
}

gboolean trash_info_is_directory(TrashInfo *self) {
    return self->is_directory;
}

GDateTime *trash_info_get_deletion_time(TrashInfo *self) {
    return g_date_time_ref(self->deleted_time);
}
