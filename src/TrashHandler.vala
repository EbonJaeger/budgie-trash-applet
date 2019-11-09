namespace TrashApplet {

    /**
     * TrashHandler is the class that handles the finding and creation
     * of TrashStores.
     *
     * It holds a reference to each TrashStore, and watches for
     * mounts and unmounts in order to keep the UI updated via signals.
     *
     * TrashHandler will always create a TrashStore for the user's main
     * trash bin on creation.
     */
    public class TrashHandler {
        private Applet applet;
        private HashTable<string, TrashStore> trash_stores;
        private VolumeMonitor monitor;
        private int uid;

        /* Signals */

        /**
         * Called when a device with a trash bin has been mounted.
         * 
         * @param trash_store The TrashStore for the added mount
         */
        public signal void trash_store_added(TrashStore trash_store);

        /**
         * Called when a device has been unmounted.
         * 
         * @param trash_store The TrashStore that was just unmounted
         */
        public signal void trash_store_removed(TrashStore trash_store);

        public TrashHandler(Applet applet) {
            this.applet = applet;
            this.trash_stores = new HashTable<string, TrashStore>(str_hash, str_equal);

            // Set up the main trash store
            var default_trash_dir = File.new_for_path(GLib.Environment.get_user_data_dir() + "/Trash/files");
            var default_info_dir = File.new_for_path(GLib.Environment.get_user_data_dir() + "/Trash/info");
            var icon = Icon.new_for_string("drive-harddisk-symbolic");
            var default_trash_store = new TrashStore(applet, default_trash_dir, default_info_dir, "This PC", null, icon);
            this.trash_stores.insert("default", default_trash_store);

            // Get the current user's UID to get their trash directory on removable drives
            this.uid = get_user_id();

            // Initialize any other trash stores that are currently present
            this.monitor = VolumeMonitor.get();
            search_current_mounts(monitor);

            // Connect signals for future mount operations
            monitor.mount_added.connect(process_mount_added);
            monitor.mount_removed.connect(process_mount_removed);
        }

        /**
         * Process a connect mount to see if it has a trash bin.
         * 
         * Trash bins on external drives reside in a folder following
         * the format ".Trash-$UID", where $UID is the current user's
         * UID. If this directory is found, we'll create a TrashStore
         * for it, and emit the `trash_store_added` signal.
         * 
         * @param mount The Mount that was added
         */
        private void process_mount_added(Mount mount) {
            var location = mount.get_default_location();
            var attributes = FileAttribute.STANDARD_NAME + "," + FileAttribute.STANDARD_TYPE;
            var trash_dir_name = ".Trash-%d".printf(uid);

            try {
                var enumerator = location.enumerate_children(attributes, 0);

                FileInfo info;
                while ((info = enumerator.next_file()) != null) { // Iterate through all files in the directory
                    if (info.get_file_type() == FileType.DIRECTORY && info.get_name() == trash_dir_name) { // Child is a trash bin directory
                        var dir = File.new_for_path(location.get_path() + "/" + info.get_name());
                        var trash_dir = File.new_for_path(dir.get_path() + "/files");
                        var info_dir = File.new_for_path(dir.get_path() + "/info");
                        var trash_store = new TrashStore(applet, trash_dir, info_dir, mount.get_name(), mount.get_default_location().get_path(), mount.get_symbolic_icon());
                        this.trash_stores.insert(mount.get_name(), trash_store);
                        trash_store_added(trash_store);
                        return;
                    }
                }
            } catch (Error e) {
                warning("Error while searching for trash bin in '%s': %s", mount.get_name(), e.message);
                applet.show_notification("Unable to mount trash bin", "Unable to mount trash for '%s': %s".printf(mount.get_name(), e.message));
                return;
            }
        }

        private void process_mount_removed(Mount mount) {
            if (trash_stores.contains(mount.get_name())) {
                var trash_store = trash_stores.get(mount.get_name());
                trash_store_removed(trash_store);
                trash_stores.remove(mount.get_name());
            }
        }

        public void get_current_trash_items() {
            trash_stores.get_values().foreach((entry) => {
                entry.get_current_trash_items();
            });
        }

        public List<weak TrashStore> get_trash_stores() {
            return trash_stores.get_values();
        }

        /**
         * Get the current user's UID by calling out to a command line utility.
         * 
         * This is hacky and I hate it. I would much rather use accountsservice's UserManager,
         * but that has its own set of problems; mainly waiting for it to load users
         * so it can actually be used.
         * 
         * @returns The UID of the current user, or -1
         */
        private int get_user_id() {
            var uid = -1;
            var cmd = "id -u";

            int status;
            string std_out, std_err;
            try {
                Process.spawn_command_line_sync(cmd, out std_out, out std_err, out status);
            } catch (SpawnError e) {
                warning("Unable to get current user's UID: %s", e.message);
                return -1;
            }

            if ((std_out != null) && (std_out.length > 0)){
                uid = int.parse(std_out);
            }

            return uid;
        }

        private void search_current_mounts(VolumeMonitor monitor) {
            List<Mount> mounts = monitor.get_mounts();
            foreach (var mount in mounts) { // Iterate through all mounted mounts
                process_mount_added(mount);
            }
        }
    } // End class
} // End namespace
