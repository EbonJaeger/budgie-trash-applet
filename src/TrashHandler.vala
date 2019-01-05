namespace TrashApplet { 

    public class TrashHandler {

        private File trash_dir;
        private File info_dir;
        private FileMonitor trash_monitor;

        private TrashPopover popover;

        public TrashHandler(TrashPopover popover) {
            this.trash_dir = File.new_for_path(GLib.Environment.get_user_data_dir() + "/Trash/files");
            this.info_dir = File.new_for_path(GLib.Environment.get_user_data_dir() + "/Trash/info");
            this.popover = popover;

            try {
                this.trash_monitor = trash_dir.monitor_directory(FileMonitorFlags.WATCH_MOVES);
            } catch (Error e) {
                warning("Unable to create TrashHandler: %s", e.message);
                return;
            }

            get_current_trash_items();

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

                    popover.add_trash_item(file_path, file_name, file_icon);
                    break;
                case FileMonitorEvent.MOVED_OUT: // A file was moved out of the trash
                    var file_name = file.get_basename();
                    popover.remove_trash_item(file_name);
                    break;
                default: // We don't care about anything else
                    break;
            }
        }

        private void get_current_trash_items() {
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

                    popover.add_trash_item(path, info.get_name(), info.get_icon());
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
    } // End class
} // End namespace
