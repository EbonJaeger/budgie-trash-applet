#include "trash_manager.h"

enum {
    SIGNAL_TRASH_BIN_ADDED,
    SIGNAL_TRASH_BIN_REMOVED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

struct _TrashManager {
    GObject parent_instance;

    GVolumeMonitor *volume_monitor;
    gint uid;
};

G_DEFINE_FINAL_TYPE(TrashManager, trash_manager, G_TYPE_OBJECT)

static void mount_added(GMount *mount, TrashManager *self) {
    TrashStore *bin;
    g_autofree gchar *mount_root, *mount_name, *base_trash_path, *trash_dir_name, *trash_dir_path, *uid_str;
    g_autoptr(GFile) location, base_trash_dir, trash_dir;
    g_autoptr(GIcon) icon;

    location = g_mount_get_default_location(mount);
    mount_root = g_file_get_path(location);
    base_trash_path = g_build_path(G_DIR_SEPARATOR_S, mount_root, ".Trash", NULL);
    base_trash_dir = g_file_new_for_path(base_trash_path);

    // Check if the base trash dir exists
    if (g_file_query_exists(base_trash_dir, NULL)) {
        // It exists, so set the trash directory path to $topdir/.Trash/$uid
        uid_str = g_strdup_printf("%d", self->uid);
        trash_dir_path = g_build_path(G_DIR_SEPARATOR_S, base_trash_path, uid_str, NULL);
    } else {
        // It doesn't exist, so set the trash directory path to $topdir/.Trash-$uid
        trash_dir_name = g_strdup_printf(".Trash-%d", self->uid);
        trash_dir_path = g_build_path(G_DIR_SEPARATOR_S, mount_root, trash_dir_name, NULL);
    }

    trash_dir = g_file_new_for_path(trash_dir_path);

    // Check if the final trash dir exists
    if (!g_file_query_exists(trash_dir, NULL)) {
        g_debug("Mount '%s' added, but no trashbin was found", g_mount_get_name(mount));
        return;
    }

    // Create the trash bin object
    mount_name = g_mount_get_name(mount);
    icon = g_mount_get_icon(mount);
    bin = trash_store_new_with_path(mount_name, icon, trash_dir_path);

    // Call our trash-bin-added signal
    g_signal_emit(self, signals[SIGNAL_TRASH_BIN_ADDED], 0, bin);
}

static void mount_removed(__attribute__((unused)) GVolumeMonitor *monitor, GMount *mount, TrashManager *self) {
    g_autofree gchar *name = NULL;

    name = g_mount_get_name(mount);

    g_signal_emit(self, signals[SIGNAL_TRASH_BIN_REMOVED], 0, g_strdup(name));
}

/**
 * Handles iterating over a list of mounts. This just calls `mount_added` for each mount.
 */
static void add_mount(__attribute__((unused)) GVolumeMonitor *monitor, GMount *mount, TrashManager *self) {
    mount_added(mount, self);
}

static void trash_manager_constructed(GObject *object) {
    TrashManager *self;
    g_autoptr(GList) mounts;

    self = TRASH_MANAGER(object);

    self->volume_monitor = g_volume_monitor_get();
    mounts = g_volume_monitor_get_mounts(self->volume_monitor);
    g_list_foreach(mounts, (GFunc) mount_added, self);

    g_signal_connect(self->volume_monitor, "mount-added", G_CALLBACK(add_mount), self);
    g_signal_connect(self->volume_monitor, "mount-removed", G_CALLBACK(mount_removed), self);
}

static void trash_manager_finalize(GObject *object) {
    TrashManager *self;

    self = TRASH_MANAGER(object);

    g_object_unref(self->volume_monitor);

    G_OBJECT_CLASS(trash_manager_parent_class)->finalize(object);
}

static void trash_manager_class_init(TrashManagerClass *klass) {
    GObjectClass *class;

    class = G_OBJECT_CLASS(klass);

    class->constructed = trash_manager_constructed;
    class->finalize = trash_manager_finalize;

    // Signals

    signals[SIGNAL_TRASH_BIN_ADDED] = g_signal_new(
        "trash-bin-added",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL, NULL, NULL,
        G_TYPE_NONE,
        1,
        G_TYPE_POINTER
    );

    signals[SIGNAL_TRASH_BIN_REMOVED] = g_signal_new(
        "trash-bin-removed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL, NULL, NULL,
        G_TYPE_NONE,
        1,
        G_TYPE_POINTER
    );
}

static void trash_manager_init(TrashManager *self) {
    self->uid = getuid();
}

TrashManager *trash_manager_new(void) {
    return g_object_new(TRASH_TYPE_MANAGER, NULL);
}
