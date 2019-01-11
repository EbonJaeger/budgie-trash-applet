namespace TrashApplet { 

    public class TrashHandler {

        private File trash_dir;
        private File info_dir;
        private FileMonitor trash_monitor;

        private int trash_count = 0;

        /* Signals */
        public signal void trash_added(string file_name, string file_path, GLib.Icon file_icon);
        public signal void trash_removed(string file_name, bool is_empty);

        public TrashHandler() {
            this.trash_dir = File.new_for_path(GLib.Environment.get_user_data_dir() + "/Trash/files");
            this.info_dir = File.new_for_path(GLib.Environment.get_user_data_dir() + "/Trash/info");

            try {
                this.trash_monitor = trash_dir.monitor_directory(FileMonitorFlags.WATCH_MOVES);
            } catch (Error e) {
                warning("Unable to create TrashHandler: %s", e.message);
                return;
            }

            this.trash_monitor.changed.connect(handle_trash_changed);
        }

        private void handle_trash_changed(File file, File? other_file, FileMonitorEvent event) {
            switch (event) {
                case FileMonitorEvent.MOVED_IN: // A file was just added to the trash
                    var file_name = file.get_basename();
                    var file_path = get_path_from_trashinfo(file_name);
                    var attributes = FileAttribute.STANDARD_ICON;

                    GLib.Icon file_icon = null;
                    try {
                        var file_info = file.query_info(attributes, FileQueryInfoFlags.NONE);
                        file_icon = file_info.get_icon();
                    } catch (Error e) {
                        warning("Unable to get icon from file info for file '%s': %s", file_name, e.message);
                        break;
                    }

                    trash_count++;
                    trash_added(file_name, file_path, file_icon);
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

        public void get_current_trash_items() {
            try {
                var attributes = FileAttribute.STANDARD_NAME + "," + FileAttribute.STANDARD_ICON;
                var enumerator = trash_dir.enumerate_children(attributes, 0);

                FileInfo info;
                while ((info = enumerator.next_file()) != null) { // Iterate through all files in the trash bin
                    var path = get_path_from_trashinfo(info.get_name());

                    if (path == null) { // Ensure that we do indeed have a path
                        warning("Unable to get the path for %s", info.get_name());
                        continue;
                    }

                    trash_count++;
                    trash_added(info.get_name(), path, info.get_icon());
                }
            } catch (Error e) {
                warning("Unable to create trash item: %s", e.message);
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

                if (info.get_type() == FileType.DIRECTORY) { // Found a nested directory
                    delete_directory(child);
                } else {
                    child.delete();
                }
            }
        }

        public void restore_file(string file_name, string restore_path) {
            var file = File.new_for_path(trash_dir.get_path() + "/" + file_name);
            var info_file = File.new_for_path(info_dir.get_path() + "/" + file_name + ".trashinfo");
            var destination = File.new_for_path(restore_path);

            try {
                file.move(destination, FileCopyFlags.NONE);
                info_file.delete();
            } catch (Error e) {
                warning("Unable to restore '%s' to '%s': %s", file_name, restore_path, e.message);
            }
        }
    } // End class
} // End namespace
