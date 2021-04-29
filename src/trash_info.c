#include "trash_info.h"

enum {
    PROP_EXP_0,
    PROP_RESTORE_PATH,
    PROP_DELETION_TIME,
    N_EXP_PROPERTIES
};

static GParamSpec *info_props[N_EXP_PROPERTIES] = {
    NULL,
};

struct _TrashInfo {
    GObject parent_instance;

    gchar *restore_path;
    GDateTime *deleted_time;
};

struct _TrashInfoClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE(TrashInfo, trash_info, G_TYPE_OBJECT);

static void trash_info_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec);
static void trash_info_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec);

static void trash_info_class_init(TrashInfoClass *klazz) {
    GObjectClass *class = G_OBJECT_CLASS(klazz);
    class->get_property = trash_info_get_property;
    class->set_property = trash_info_set_property;

    info_props[PROP_RESTORE_PATH] = g_param_spec_string(
        "restore-path",
        "Restore path",
        "Path pointing to where this item came from prior to deletion",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    info_props[PROP_DELETION_TIME] = g_param_spec_gtype(
        "deletion-time",
        "Deletion time",
        "Timestamp when this item was deleted",
        G_TYPE_NONE,
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    g_object_class_install_properties(class, N_EXP_PROPERTIES, info_props);
}

static void trash_info_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    TrashInfo *self = TRASH_INFO(obj);

    switch (prop_id) {
        case PROP_RESTORE_PATH:
            g_value_set_string(val, self->restore_path);
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
        case PROP_RESTORE_PATH:
            trash_info_set_restore_path(self, g_strdup(g_value_get_string(val)));
            break;
        case PROP_DELETION_TIME:
            trash_info_set_deletion_time(self, g_date_time_ref((GDateTime *) g_value_get_gtype(val)));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_info_init(TrashInfo *self) {
    self->restore_path = NULL;
    self->deleted_time = NULL;
}

TrashInfo *trash_info_new_from_file(GFile *file) {
    return trash_info_new_from_file_with_prefix(file, NULL);
}

TrashInfo *trash_info_new_from_file_with_prefix(GFile *file, gchar *prefix) {
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, &err);
    if (!input_stream) {
        return NULL;
    }

    // Seek to the Path line
    g_seekable_seek(G_SEEKABLE(input_stream), TRASH_INFO_PATH_OFFSET, G_SEEK_SET, NULL, NULL);

    // Read the file contents and extract the line containing the restore path
    g_autofree gchar *buffer = (gchar *) malloc(1024 * sizeof(gchar));
    gssize read;
    while ((read = g_input_stream_read(G_INPUT_STREAM(input_stream), buffer, 1024, NULL, &err))) {
        buffer[read] = '\0';
    }

    g_input_stream_close(G_INPUT_STREAM(input_stream), NULL, NULL);

    gchar **lines = g_strsplit(buffer, "\n", 2);
    g_autofree gchar *restore_path = substring(lines[0], TRASH_INFO_PATH_PREFIX_OFFSET, strlen(lines[0]));
    if (prefix) {
        restore_path = g_strconcat(prefix, G_DIR_SEPARATOR_S, restore_path, NULL);
    }
    g_autofree gchar *deletion_time_str = g_strstrip(substring(lines[1], TRASH_INFO_DELETION_DATE_PREFIX_OFFSET, strlen(lines[1])));
    g_autoptr(GTimeZone) tz = g_time_zone_new_local();
    g_autoptr(GDateTime) deletion_time = g_date_time_new_from_iso8601((const gchar *) deletion_time_str, tz);
    g_strfreev(lines);

    return g_object_new(TRASH_TYPE_INFO,
                        "restore-path", restore_path,
                        "deletion-time", deletion_time,
                        NULL);
}

gchar *trash_info_get_restore_path(TrashInfo *self) {
    return g_strdup(self->restore_path);
}

GDateTime *trash_info_get_deletion_time(TrashInfo *self) {
    return g_date_time_ref(self->deleted_time);
}

gchar *trash_info_format_deletion_time(TrashInfo *self) {
    return g_date_time_format(self->deleted_time, "%Y-%m-%d %H:%M %Z");
}

void trash_info_set_restore_path(TrashInfo *self, gchar *path) {
    gchar *path_clone = g_strdup(path);

    if (path_clone == NULL || strcmp(path_clone, "") == 0) {
        return;
    }

    // Free existing path if it's different
    if ((self->restore_path != NULL) && strcmp(self->restore_path, path_clone) != 0) {
        g_free(self->restore_path);
    }

    self->restore_path = path_clone;

    g_object_notify_by_pspec(G_OBJECT(self), info_props[PROP_RESTORE_PATH]);
}

void trash_info_set_deletion_time(TrashInfo *self, GDateTime *timestamp) {
    GDateTime *time_clone = g_date_time_ref(timestamp);

    if (self->deleted_time != NULL && g_date_time_equal(self->deleted_time, time_clone)) {
        g_date_time_unref(self->deleted_time);
        if (self->deleted_time != NULL) {
            g_critical("Deletion timestamp still has an active ref");
        }
    }

    self->deleted_time = time_clone;

    g_object_notify_by_pspec(G_OBJECT(self), info_props[PROP_DELETION_TIME]);
}
