namespace TrashApplet { 
    
    /**
     * TrashStore represents a trash bin on the user's system.
     * 
     * Removable drives may have their own trash locations that should be tracked,
     * along with the user's main trash bin.
     */
    public class TrashStore {
        private File trash_dir;
        private File info_dir;

        private string drive_name;
        private string? drive_path;
        private Icon drive_icon;

        private FileMonitor trash_monitor;

        private int trash_count = 0;

        /* Signals */
        public signal void trash_added(string file_name, string file_path, GLib.Icon file_icon, bool is_directory);
        public signal void trash_removed(string file_name, bool is_empty);

        public TrashStore(File trash_dir, File info_dir, string drive_name, string? drive_path, Icon drive_icon) {
            this.trash_dir = trash_dir;
            this.info_dir = info_dir;
            this.drive_name = drive_name;
            this.drive_path = drive_path;
            this.drive_icon = drive_icon;

            try {
                this.trash_monitor = trash_dir.monitor_directory(FileMonitorFlags.WATCH_MOVES);
            } catch (Error e) {
                warning("Unable to create a TrashStore: %s", e.message);
                return;
            }

            this.trash_monitor.changed.connect(handle_trash_changed);
        }

        /**
         * Gets all of the current items in this store's trash bin.
         * 
         * A `trash_added` signal is emitted for each found trash item so
         * the UI can add each one accordingly.
         */
        public void get_current_trash_items() {
            try {
                var attributes = FileAttribute.STANDARD_NAME + "," + FileAttribute.STANDARD_ICON + "," + FileAttribute.STANDARD_TYPE;
                var enumerator = trash_dir.enumerate_children(attributes, 0);

                FileInfo info;
                while ((info = enumerator.next_file()) != null) { // Iterate through all files in the trash bin
                    var path = get_path_from_trashinfo(info.get_name());

                    if (path == null) { // Ensure that we do indeed have a path
                        warning("Unable to get the path for %s", info.get_name());
                        continue;
                    }

                    bool is_directory = false;
                    if (info.get_file_type() == FileType.DIRECTORY) {
                        is_directory = true;
                    }

                    trash_count++;
                    trash_added(info.get_name(), path, info.get_icon(), is_directory);
                }
            } catch (Error e) {
                warning("Unable to create trash item: %s", e.message);
            }
        }

        public string get_drive_name() {
            return this.drive_name;
        }

        public Icon get_drive_icon() {
            return this.drive_icon;
        }

        /**
         * Delete a file permanently from the trash.
         * 
         * If the file is a directory, it will be recursively deleted.
         * 
         * @param file_name The name of the file to delete
         */
        public void delete_file(string file_name) {
            File file = File.new_for_path(trash_dir.get_path() + "/" + file_name);
            File info_file = File.new_for_path(info_dir.get_path() + "/" + file_name + ".trashinfo");

            // First, check if this is a file or directory
            FileType type = file.query_file_type(FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
            if (type == FileType.DIRECTORY) { // Item is a directory, and directories must be empty to delete them
                try {
                    delete_directory(file);
                } catch (Error e) {
                    warning("Unable to delete directory '%s' in trash: %s", file_name, e.message);
                    return;
                }
            }

            try {
                file.delete();
                info_file.delete();
            } catch (Error e) {
                warning("Unable to delete '%s': %s", file_name, e.message);
            }
        }

        /**
         * Recursively delete a directory.
         */
        private void delete_directory(File dir) throws Error {
            var attributes = FileAttribute.STANDARD_NAME + "," + FileAttribute.STANDARD_TYPE;
            var enumerator = dir.enumerate_children(attributes, 0);

            FileInfo info;
            while ((info = enumerator.next_file()) != null) { // Iterate through all files in the directory
                var child = File.new_for_path(dir.get_path() + "/" + info.get_name());

                if (info.get_file_type() == FileType.DIRECTORY) { // Found a nested directory
                    delete_directory(child);
                    child.delete();
                } else { // Not a nested directory, just delete the file
                    child.delete();
                }
            }
        }

        public void restore_file(string file_name, string restore_path) {
            var file = File.new_for_path(trash_dir.get_path() + "/" + file_name);
            var info_file = File.new_for_path(info_dir.get_path() + "/" + file_name + ".trashinfo");
            
            File destination;
            if (drive_path != null) {
                destination = File.new_for_path(drive_path + "/" + restore_path);
            } else {
                destination = File.new_for_path(restore_path);
            }

            try {
                file.move(destination, FileCopyFlags.NONE);
                info_file.delete();
            } catch (Error e) {
                warning("Unable to restore '%s' to '%s': %s", file_name, restore_path, e.message);
            }
        }

        private void handle_trash_changed(File file, File? other_file, FileMonitorEvent event) {
            switch (event) {
                case FileMonitorEvent.MOVED_IN: // A file was just added to the trash
                    var file_name = file.get_basename();
                    var file_path = get_path_from_trashinfo(file_name);
                    var attributes = FileAttribute.STANDARD_ICON + "," + FileAttribute.STANDARD_TYPE;

                    GLib.Icon file_icon = null;
                    bool is_directory = false;
                    try {
                        var file_info = file.query_info(attributes, FileQueryInfoFlags.NONE);
                        file_icon = file_info.get_icon();

                        if (file_info.get_file_type() == FileType.DIRECTORY) {
                            is_directory = true;
                        }
                    } catch (Error e) {
                        warning("Unable to get icon from file info for file '%s': %s", file_name, e.message);
                        break;
                    }

                    trash_count++;
                    trash_added(file_name, file_path, file_icon, is_directory);
                    break;
                case FileMonitorEvent.MOVED_OUT: // A file was moved out of the trash
                    var file_name = file.get_basename();
                    trash_count--;
                    trash_removed(file_name, (trash_count == 0));
                    break;
                case FileMonitorEvent.DELETED: // A file was permanently deleted from the trash
                    var file_name = file.get_basename();
                    trash_count--;
                    trash_removed(file_name, (trash_count == 0));
                    break;
                default: // We don't care about anything else
                    break;
            }
        }

        private string? get_path_from_trashinfo(string file_name) {
            File info_file = File.new_for_path(info_dir.get_path() + "/" + file_name + ".trashinfo");
            string line = null;
            string path = null;

            try {
                var dis = new DataInputStream(info_file.read());
                while ((line = dis.read_line()) != null) { // Read the lines of the .trashinfo file
                    if (!line.has_prefix("Path=")) { // If its not the path line, skip it
                        continue;
                    }

                    path = line.substring(5); // This cuts out the Path= prefix in the line
                    break;
                }
            } catch (Error e) {
                warning("Error reading data from .trashinfo: %s", e.message);
            }

            return path;
        }
    }
}
