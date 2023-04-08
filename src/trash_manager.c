#include "trash_manager.h"

enum {
    SIGNAL_TRASH_ADDED,
    SIGNAL_TRASH_REMOVED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

struct _TrashManager {
    GObject parent_instance;

    GFileMonitor *trash_monitor;
    gint file_count;
};

G_DEFINE_FINAL_TYPE(TrashManager, trash_manager, G_TYPE_OBJECT)

static void trash_manager_finalize(GObject *object) {
    TrashManager *self;

    self = TRASH_MANAGER(object);

    if (self->trash_monitor) {
        g_object_unref(self->trash_monitor);
    }

    G_OBJECT_CLASS(trash_manager_parent_class)->finalize(object);
}

static void trash_manager_class_init(TrashManagerClass *klass) {
    GObjectClass *class;

    class = G_OBJECT_CLASS(klass);
    class->finalize = trash_manager_finalize;

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
}

static void trash_query_info_cb(GObject *source, GAsyncResult *result, gpointer user_data) {
    TrashManager *self = user_data;
    GFileInfo *info;
    const gchar *uri;
    const gchar *unescaped_uri;
    TrashInfo *trash_info;

    info = g_file_query_info_finish(G_FILE(source), result, NULL);

    g_return_if_fail(G_IS_FILE_INFO(info));

    uri = g_file_get_uri(G_FILE(source));
    unescaped_uri = g_uri_unescape_string(uri, NULL);
    trash_info = trash_info_new(info, unescaped_uri);

    self->file_count++;
    g_signal_emit(self, signals[SIGNAL_TRASH_ADDED], 0, trash_info);
}

static void file_changed(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event, TrashManager *self) {
    (void) monitor;
    (void) other_file;

    const gchar *uri;
    const gchar *unescaped_uri;

    switch (event) {
        case G_FILE_MONITOR_EVENT_MOVED_IN:
        case G_FILE_MONITOR_EVENT_CREATED:
            g_file_query_info_async(
                file,
                TRASH_FILE_ATTRIBUTES,
                G_FILE_QUERY_INFO_NONE,
                G_PRIORITY_DEFAULT,
                NULL,
                trash_query_info_cb,
                self
            );
            break;
        case G_FILE_MONITOR_EVENT_MOVED_OUT:
        case G_FILE_MONITOR_EVENT_DELETED:
            self->file_count--;
            uri = g_file_get_uri(file);
            unescaped_uri = g_uri_unescape_string(uri, NULL);
            g_signal_emit(self, signals[SIGNAL_TRASH_REMOVED], 0, unescaped_uri);
            break;
        default:
            break;
    }
}

static void trash_manager_init(TrashManager *self) {
    GFile *file;

    self->file_count = 0;

    file = g_file_new_for_uri("trash:///");

    self->trash_monitor = g_file_monitor(file, 0, NULL, NULL);

    g_signal_connect(self->trash_monitor, "changed", G_CALLBACK(file_changed), self);
}

TrashManager *trash_manager_new(void) {
    return g_object_new(TRASH_TYPE_MANAGER, NULL);
}

static void next_file_cb(gpointer data, gpointer user_data) {
    TrashManager *self = user_data;
    g_autoptr(GFileInfo) file_info;
    const gchar *file_name;
    g_autofree const gchar *uri;
    TrashInfo *trash_info;

    file_info = data;

    g_return_if_fail(G_IS_FILE_INFO(file_info));

    file_name = g_file_info_get_name(file_info);
    uri = g_strdup_printf("trash:///%s", file_name);
    trash_info = trash_info_new(file_info, uri);

    self->file_count++;
    g_signal_emit(self, signals[SIGNAL_TRASH_ADDED], 0, trash_info);
}

static void next_files_cb(GObject *source, GAsyncResult *result, gpointer user_data) {
    TrashManager *self = user_data;
    g_autoptr(GList) files;
    g_autoptr(GError) error = NULL;

    files = g_file_enumerator_next_files_finish(G_FILE_ENUMERATOR(source), result, &error);

    if (error) {
        g_critical("Error getting next files from enumerator: %s", error->message);
        g_object_unref(source); // Unref the file enumerator
        return;
    }

    if (!files) {
        g_object_unref(source); // Unref the file enumerator
        return;
    }

    g_list_foreach(files, next_file_cb, self);

    g_file_enumerator_next_files_async(G_FILE_ENUMERATOR(source), 8, G_PRIORITY_DEFAULT, NULL, next_files_cb, self);
}

static void trash_enumerate_cb(GObject *source, GAsyncResult *result, gpointer user_data) {
    TrashManager *self = user_data;
    GFileEnumerator *enumerator;
    g_autoptr(GError) error = NULL;

    enumerator = g_file_enumerate_children_finish(G_FILE(source), result, &error);

    if (!G_IS_FILE_ENUMERATOR(enumerator)) {
        g_critical("Error getting trash enumerator: %s", error->message);
        return;
    }

    g_file_enumerator_next_files_async(enumerator, 8, G_PRIORITY_DEFAULT, NULL, next_files_cb, self);
}

/**
 * Scan the trash bin for items. The `trash-added` signal will be called for each
 * item found in the bin.
 *
 * The files are enumerated asynchronously.
 */
void trash_manager_scan_items(TrashManager *self) {
    g_autoptr(GFile) file;

    file = g_file_new_for_uri("trash:///");

    g_file_enumerate_children_async(
        file,
        TRASH_FILE_ATTRIBUTES,
        G_FILE_QUERY_INFO_NONE,
        G_PRIORITY_DEFAULT,
        NULL,
        trash_enumerate_cb,
        self
    );
}
