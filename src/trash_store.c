#include "trash_store.h"

enum {
    SIGNAL_TRASH_ADDED,
    SIGNAL_TRASH_REMOVED,
    N_SIGNALS
};

enum {
    PROP_COUNT,
    PROP_DEFAULT,
    PROP_ICON,
    PROP_NAME,
    PROP_PATH,
    N_PROPS
};

static guint signals[N_SIGNALS];
static GParamSpec *props[N_PROPS];

struct _TrashStore {
    GObject parent_instance;

    GFileMonitor *file_monitor;

    gint file_count;
    gboolean is_default;
    GIcon *icon;
    const gchar *name;
    const gchar *path;
};

G_DEFINE_TYPE(TrashStore, trash_store, G_TYPE_OBJECT);

static void trash_store_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *spec) {
    TrashStore *self;
    GIcon *icon;
    
    self = TRASH_STORE(object);

    switch (prop_id) {
        case PROP_COUNT:
            g_value_set_int(value, trash_store_get_count(self));
            break;
        case PROP_DEFAULT:
            g_value_set_boolean(value, trash_store_is_default(self));
            break;
        case PROP_ICON:
            icon = trash_store_get_icon(self);
            g_value_set_variant(value, g_icon_serialize(icon));
            break;
        case PROP_NAME:
            g_value_set_string(value, trash_store_get_name(self));
            break;
        case PROP_PATH:
            g_value_set_string(value, trash_store_get_path(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, spec);
            break;
    }
}

static void trash_store_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *spec) {
    TrashStore *self;
    GVariant *raw_icon;
    
    self = TRASH_STORE(object);

    switch (prop_id) {
        case PROP_COUNT:
            self->file_count = g_value_get_int(value);
            break;
        case PROP_DEFAULT:
            self->is_default = g_value_get_boolean(value);
            break;
        case PROP_ICON:
            raw_icon = g_value_get_variant(value);
            self->icon = g_icon_deserialize(raw_icon);
            break;
        case PROP_NAME:
            self->name = g_value_get_string(value);
            break;
        case PROP_PATH:
            self->path = g_value_get_string(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, spec);
            break;
    }
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
        escaped = g_uri_escape_string(file_name, NULL, TRUE);
    } else {
        /*
         * This abomination is because GLib does some really weird things with escaping characters
         * in paths for mounts in trash URIs. Specifically, forward slashes are back slashes and
         * spaces are escaped. Then it get's even more special because for some reason at least part
         * of it gets escaped *before* that happens, because spaces end up as `%2520` (the % gets
         * escaped). So, to get a valid URI to a trash file, we have to emulate that behavior.
         */

        unescaped = g_string_new(g_strdup_printf("%s\\%s", self->path, file_name));
        g_string_replace(unescaped, "/", "\\", 0);
        g_string_replace(unescaped, " ", "%20", 0);
        escaped = g_uri_escape_string(unescaped->str, NULL, TRUE);
    }

    path = g_strdup_printf("/%s", escaped);

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
 * G_FILE_MONITOR_EVENT_DELETED events.
 */
static void
handle_monitor_event(
    __attribute__((unused)) GFileMonitor *monitor,
    GFile *file,
    __attribute__((unused)) GFile *other_file,
    GFileMonitorEvent event_type,
    TrashStore *self
) {
    g_autoptr(GUri) uri;
    g_autofree gchar *uri_string = NULL;
    g_autoptr(GError) error = NULL;
    TrashInfo *trash_info;

    uri = uri_for_file(self, g_file_get_basename(file));
    g_return_if_fail(uri != NULL);
    uri_string = g_uri_to_string(uri);

    switch (event_type) {
        case G_FILE_MONITOR_EVENT_MOVED_IN:
        case G_FILE_MONITOR_EVENT_CREATED:
            trash_info = trash_info_new(uri_string, &error);

            if (!TRASH_IS_INFO(trash_info)) {
                g_warning("%s:%d: Error making trash info for URI '%s': %s", __BASE_FILE__, __LINE__, uri_string, error->message);
                return;
            }

            self->file_count++;
            g_signal_emit(self, signals[SIGNAL_TRASH_ADDED], 0, trash_info, NULL);
            break;
        case G_FILE_MONITOR_EVENT_MOVED_OUT:
        case G_FILE_MONITOR_EVENT_DELETED:
            self->file_count--;
            g_signal_emit(self, signals[SIGNAL_TRASH_REMOVED], 0, uri_string, NULL);
            break;
        default:
            break;
    }
}

static void trash_store_constructed(GObject *object) {
    TrashStore *self;
    g_autofree gchar *files_path;
    g_autoptr(GFile) dir;
    g_autoptr(GError) error = NULL;

    self = TRASH_STORE(object);
    
    files_path = g_build_path(G_DIR_SEPARATOR_S, self->path, "files", NULL);
    dir = g_file_new_for_path(files_path);
    self->file_monitor = g_file_monitor(dir, G_FILE_MONITOR_NONE, NULL, &error);

    if (!self->file_monitor) {
        g_critical("error monitoring directory '%s': %s", self->path, error->message);
        return;
    }

    g_signal_connect(self->file_monitor, "changed", G_CALLBACK(handle_monitor_event), self);
}

static void trash_store_finalize(GObject *object) {
    TrashStore *self;
    
    self = TRASH_STORE(object);

    g_object_unref(self->file_monitor);

    g_object_unref(self->icon);
    g_free((gchar *) self->name);
    g_free((gchar *) self->path);

    G_OBJECT_CLASS(trash_store_parent_class)->finalize(object);
}

static void trash_store_class_init(TrashStoreClass *klass) {
    GObjectClass *class;
    GIcon *icon;
    
    class = G_OBJECT_CLASS(klass);

    class->constructed = trash_store_constructed;
    class->finalize = trash_store_finalize;
    class->get_property = trash_store_get_property;
    class->set_property = trash_store_set_property;

    // Signals

    signals[SIGNAL_TRASH_ADDED] = g_signal_new(
        "trash-added",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL, NULL, NULL,
        G_TYPE_NONE,
        1,
        G_TYPE_POINTER
    );

    signals[SIGNAL_TRASH_REMOVED] = g_signal_new(
        "trash-removed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL, NULL, NULL,
        G_TYPE_NONE,
        1,
        G_TYPE_POINTER
    );

    // Properties

    props[PROP_COUNT] = g_param_spec_int(
        "count",
        "Count",
        "The current number of files in the bin",
        0,
        G_MAXINT,
        0,
        G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_DEFAULT] = g_param_spec_boolean(
        "is-default",
        "Default",
        "Whether or not this is the user's default trashbin",
        FALSE,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    icon = g_icon_new_for_string("drive-harddisk-symbolic", NULL);
    props[PROP_ICON] = g_param_spec_variant(
        "icon",
        "Icon",
        "The icon to use for display for this bin",
        G_VARIANT_TYPE_ANY,
        g_icon_serialize(icon),
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_NAME] = g_param_spec_string(
        "name",
        "Name",
        "The name of this bin",
        "This PC",
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    props[PROP_PATH] = g_param_spec_string(
        "path",
        "Path",
        "The path to this bin",
        g_build_path(G_DIR_SEPARATOR_S, g_get_user_data_dir(), "Trash", "files", NULL),
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
    );

    g_object_class_install_properties(class, N_PROPS, props);
}

static void trash_store_init(TrashStore *self) {
    (void) self;
}

TrashStore *trash_store_new_default(void) {
    return g_object_new(TRASH_TYPE_STORE, "is-default", TRUE, NULL);
}

TrashStore *trash_store_new_with_path(gchar *drive_name, GIcon *icon, gchar *trash_path) {
    return g_object_new(TRASH_TYPE_STORE, "icon", g_icon_serialize(g_object_ref(icon)), "name", g_strdup(drive_name), "path", g_strdup(trash_path), NULL);
}

GList *trash_store_get_items(TrashStore *self, GError *error) {
    GList *items = NULL;
    g_autoptr(GFile) directory;
    g_autoptr(GFileEnumerator) enumerator;
    g_autoptr(GFileInfo) current_file = NULL;

    self->file_count = 0;
    directory = g_file_new_for_path(self->path);
    enumerator = g_file_enumerate_children(directory, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, &error);

    if (!G_IS_FILE_ENUMERATOR(enumerator)) {
        g_critical("%s:%d: Error getting file enumerator for trash files in '%s': %s", __BASE_FILE__, __LINE__, self->path, error->message);
        return items;
    }

    // Iterate over the directory's children and append each file name to a list
    while ((current_file = g_file_enumerator_next_file(enumerator, NULL, &error))) {
        g_autoptr(GUri) uri;
        TrashInfo *trash_info;
        g_autoptr(GError) info_error = NULL;

        uri = uri_for_file(self, g_file_info_get_name(current_file));
        trash_info = trash_info_new(g_uri_to_string(uri), &info_error);

        if (!TRASH_IS_INFO(trash_info)) {
            g_warning("%s:%d: Error making trash info for URI '%s': %s", __BASE_FILE__, __LINE__, g_uri_to_string(uri), info_error->message);
            continue;
        }

        items = g_list_append(items, trash_info);
        self->file_count++;
    }

    g_file_enumerator_close(enumerator, NULL, NULL);
    return items;
}

gint trash_store_get_count(TrashStore *self) {
    return self->file_count;
}

GIcon *trash_store_get_icon(TrashStore *self) {
    return g_object_ref(self->icon);
}

const gchar *trash_store_get_name(TrashStore *self) {
    return g_strdup(self->name);
}

const gchar *trash_store_get_path(TrashStore *self) {
    return g_strdup(self->path);
}

gboolean trash_store_is_default(TrashStore *self) {
    return self->is_default;
}
