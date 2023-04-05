#include "trash_info.h"
#include "utils.h"

enum {
    PROP_NAME,
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
    const gchar *uri;
    const gchar *restore_path;

    GIcon *icon;

    goffset size;
    gboolean is_directory;

    GDateTime *deleted_time;
};

G_DEFINE_TYPE(TrashInfo, trash_info, G_TYPE_OBJECT);

static void trash_info_finalize(GObject *obj) {
    g_return_if_fail(obj != NULL);
    g_return_if_fail(TRASH_IS_INFO(obj));

    TrashInfo *self = TRASH_INFO(obj);

    g_free((gchar *) self->name);
    g_free((gchar *) self->uri);
    g_free((gchar *) self->restore_path);

    g_date_time_unref(self->deleted_time);

    G_OBJECT_CLASS(trash_info_parent_class)->finalize(obj);
}

static void trash_info_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    TrashInfo *self = TRASH_INFO(obj);

    switch (prop_id) {
        case PROP_NAME:
            g_value_set_string(val, self->name);
            break;
        case PROP_URI:
            g_value_set_string(val, self->uri);
            break;
        case PROP_RESTORE_PATH:
            g_value_set_string(val, self->restore_path);
            break;
        case PROP_ICON:
            g_value_set_variant(val, g_icon_serialize(self->icon));
            break;
        case PROP_SIZE:
            g_value_set_uint64(val, self->size);
            break;
        case PROP_IS_DIR:
            g_value_set_boolean(val, self->is_directory);
            break;
        case PROP_DELETION_TIME:
            g_value_set_gtype(val, (GType) self->deleted_time);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_info_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    TrashInfo *self = TRASH_INFO(obj);

    switch (prop_id) {
        case PROP_NAME:
            trash_info_set_file_name(self, g_value_get_string(val));
            break;
        case PROP_URI:
            trash_info_set_file_uri(self, g_value_get_string(val));
            break;
        case PROP_RESTORE_PATH:
            trash_info_set_restore_path(self, g_value_get_string(val));
            break;
        case PROP_ICON:
            trash_info_set_icon(self, g_icon_deserialize(g_value_get_variant(val)));
            break;
        case PROP_SIZE:
            trash_info_set_file_size(self, g_value_get_uint64(val));
            break;
        case PROP_IS_DIR:
            trash_info_set_is_directory(self, g_value_get_boolean(val));
            break;
        case PROP_DELETION_TIME:
            trash_info_set_deletion_time(self, (GDateTime *) g_value_get_gtype(val));
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
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_URI] = g_param_spec_string(
        "uri",
        "file uri",
        "The URI of the trashed file",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_RESTORE_PATH] = g_param_spec_string(
        "restore-path",
        "restore path",
        "The original path to the file",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_ICON] = g_param_spec_variant(
        "icon",
        "file icon",
        "The display icon for the file",
        G_VARIANT_TYPE_ANY,
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_SIZE] = g_param_spec_uint64(
        "size",
        "file size",
        "The size of the file on disk",
        0,
        G_MAXINT64,
        0,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_IS_DIR] = g_param_spec_boolean(
        "is-dir",
        "is directory",
        "If the file is a directory or not",
        FALSE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    props[PROP_DELETION_TIME] = g_param_spec_gtype(
        "deletion-time",
        "deletion time",
        "The timestamp of when the file was deleted",
        G_TYPE_NONE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    g_object_class_install_properties(class, N_PROPS, props);
}

static void trash_info_init(__attribute__((unused)) TrashInfo *self) {}

TrashInfo *trash_info_new(const gchar *uri, GError **err) {
    g_autoptr(GFile) file = NULL;
    g_autoptr(GFileInfo) file_info = NULL;
    GIcon *icon;

    file = g_file_new_for_uri(uri);
    file_info = g_file_query_info(file, TRASH_FILE_ATTRIBUTES, G_FILE_QUERY_INFO_NONE, NULL, err);

    if (!G_IS_FILE_INFO(file_info)) {
        return NULL;
    }

    icon = g_file_info_get_icon(file_info);

    return g_object_new(
        TRASH_TYPE_INFO,
        "name", g_file_info_get_name(file_info),
        "uri", uri,
        "restore-path", g_file_info_get_attribute_byte_string(file_info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH),
        "icon", g_icon_serialize(icon),
        "size", g_file_info_get_size(file_info),
        "is-dir", (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY),
        "deletion-time", g_file_info_get_deletion_date(file_info),
        NULL);
}

/* Property getters */

const gchar *trash_info_get_name(TrashInfo *self) {
    return self->name;
}

const gchar *trash_info_get_uri(TrashInfo *self) {
    return self->uri;
}

const gchar *trash_info_get_restore_path(TrashInfo *self) {
    return self->restore_path;
}

GIcon *trash_info_get_icon(TrashInfo *self) {
    return self->icon;
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

/* Property setters */

void trash_info_set_file_name(TrashInfo *self, const gchar *value) {
    g_return_if_fail(TRASH_IS_INFO(self));

    if (!trash_utils_is_string_valid((gchar *) value)) {
        return;
    }

    if (trash_utils_is_string_valid((gchar *) self->name)) {
        g_free((gchar *) self->name);
    }

    gchar *name = g_strdup(value);

    self->name = name;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_NAME]);
}

void trash_info_set_file_uri(TrashInfo *self, const gchar *value) {
    g_return_if_fail(TRASH_IS_INFO(self));

    if (!trash_utils_is_string_valid((gchar *) value)) {
        return;
    }

    if (trash_utils_is_string_valid((gchar *) self->uri)) {
        g_free((gchar *) self->uri);
    }

    const gchar *uri = g_strdup(value);

    self->uri = uri;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_URI]);
}

void trash_info_set_restore_path(TrashInfo *self, const gchar *value) {
    g_return_if_fail(TRASH_IS_INFO(self));

    if (!trash_utils_is_string_valid((gchar *) value)) {
        return;
    }

    if (trash_utils_is_string_valid((gchar *) self->restore_path)) {
        g_free((gchar *) self->restore_path);
    }

    const gchar *restore_path = g_strdup(value);

    self->restore_path = restore_path;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_RESTORE_PATH]);
}

void trash_info_set_icon(TrashInfo *self, GIcon *icon) {
    g_return_if_fail(TRASH_IS_INFO(self));

    if (self->icon) {
        g_free(self->icon);
    }

    self->icon = icon;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_ICON]);
}

void trash_info_set_file_size(TrashInfo *self, goffset value) {
    g_return_if_fail(TRASH_IS_INFO(self));

    self->size = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_SIZE]);
}

void trash_info_set_is_directory(TrashInfo *self, gboolean value) {
    g_return_if_fail(TRASH_IS_INFO(self));

    self->is_directory = value;

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_IS_DIR]);
}

void trash_info_set_deletion_time(TrashInfo *self, GDateTime *value) {
    g_return_if_fail(TRASH_IS_INFO(self));
    g_return_if_fail(value != NULL);

    if (self->deleted_time) {
        g_date_time_unref(self->deleted_time);
    }

    self->deleted_time = g_date_time_ref(value);

    g_object_notify_by_pspec(G_OBJECT(self), props[PROP_DELETION_TIME]);
}
