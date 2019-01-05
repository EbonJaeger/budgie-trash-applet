namespace TrashApplet {

    public class TrashPopover : Budgie.Popover {

        private List<TrashItem> trash_bin_items;

        /* Widgets */
        private Gtk.Stack? stack = null;
        private Gtk.Box? main_view = null;
        private Gtk.Box? title_area = null;
        private Gtk.Label? title_label = null;
        private Gtk.Box? items_count_area = null;
        private Gtk.Label? items_count = null;
        private Gtk.FlowBox? file_box = null;
        private Gtk.Box? controls_area = null;

        private Gtk.Separator? horizontal_separator = null;

        private Gtk.Button? select_all_button = null;
        private Gtk.Button? unselect_all_button = null;
        private Gtk.Button? restore_button = null;
        private Gtk.Button? delete_button = null;

        /**
         * Constructor
         */
        public TrashPopover(Gtk.Widget? parent) {
            Object(relative_to: parent);
            width_request = 600;

            this.trash_bin_items = new List<TrashItem>();
            create_trash_items();

            /* Views */
            this.stack = new Gtk.Stack();

            this.main_view = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

            this.title_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            this.title_area.height_request = 32;
            this.title_label = new Gtk.Label("Trash");
            this.title_area.pack_start(title_label, true, true, 0);

            this.items_count_area = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            this.items_count_area.height_request = 32;
            this.items_count = new Gtk.Label("Your trash bin is currently empty!");
            this.items_count_area.pack_start(items_count, true, true, 0);

            this.file_box = new Gtk.FlowBox();
            this.file_box.height_request = 200;
            this.file_box.activate_on_single_click = true;
            this.file_box.homogeneous = true;
            this.file_box.max_children_per_line = 4;
            this.file_box.selection_mode = Gtk.SelectionMode.MULTIPLE;
            trash_bin_items.foreach((entry) => { // Add all existing items in the trash bin
                this.file_box.insert(entry, -1);
            });

            this.controls_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);

            this.select_all_button = new Gtk.Button.from_icon_name("edit-select-all-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.select_all_button.set_tooltip_text("Select all items");

            this.unselect_all_button = new Gtk.Button.from_icon_name("edit-clear-all-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.unselect_all_button.set_tooltip_text("Unselect all items");

            this.restore_button = new Gtk.Button.from_icon_name("edit-undo-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.restore_button.set_tooltip_text("Restore Items");

            this.delete_button = new Gtk.Button.from_icon_name("user-trash-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.delete_button.set_tooltip_text("Delete Items");

            this.controls_area.pack_start(select_all_button);
            this.controls_area.pack_start(unselect_all_button);
            this.controls_area.pack_start(restore_button);
            this.controls_area.pack_end(delete_button);

            this.horizontal_separator = new Gtk.Separator(Gtk.Orientation.HORIZONTAL);

            this.main_view.pack_start(title_area);
            this.main_view.pack_start(horizontal_separator);
            this.main_view.pack_start(items_count_area);
            this.main_view.pack_start(file_box);
            this.main_view.pack_start(horizontal_separator);
            this.main_view.pack_end(controls_area);

            apply_button_styles();

            this.stack.add_named(main_view, "main");

            this.stack.show_all();

            connect_signals();

            add(this.stack);
        }

        public void set_page(string page) {
            this.stack.set_visible_child_name(page);
        }

        private void apply_button_styles() {
            select_all_button.get_style_context().add_class("flat");
            unselect_all_button.get_style_context().add_class("flat");
            restore_button.get_style_context().add_class("flat");
            delete_button.get_style_context().add_class("flat");

            select_all_button.get_style_context().remove_class("button");
            unselect_all_button.get_style_context().remove_class("button");
            restore_button.get_style_context().remove_class("button");
            delete_button.get_style_context().remove_class("button");
        }

        private void create_trash_items() {
            File trash_files = File.new_for_path(GLib.Environment.get_user_data_dir() + "/Trash/files");
            string info_path = GLib.Environment.get_user_data_dir() + "/Trash/info/";

            try {
                var attributes = FileAttribute.STANDARD_NAME + "," + FileAttribute.STANDARD_ICON;
                var enumerator = trash_files.enumerate_children(attributes, 0);
                FileInfo info;
                while ((info = enumerator.next_file()) != null) { // Iterate through all files in the trash bin
                    File info_file = File.new_for_path(info_path + info.get_name() + ".trashinfo");

                    var dis = new DataInputStream(info_file.read());
                    string line = null;
                    string path = null;
                    while ((line = dis.read_line()) != null) { // Read the lines of the .trashinfo file
                        if (!line.has_prefix("Path=")) { // If its not the path line, skip it
                            continue;
                        }

                        path = line.substring(5); // This cuts out the Path= prefix in the line
                        break;
                    }

                    if (path == null) { // Ensure that we do indeed have a path
                        warning("Unable to get the path for %s", info.get_name());
                        continue;
                    }

                    TrashItem trash_item = new TrashItem(info, path);
                    this.trash_bin_items.append(trash_item);
                }
            } catch (Error e) {
                warning("Unable to create trash item: %s", e.message);
            }
        }

        private void connect_signals() {
            this.select_all_button.clicked.connect(() => { // Select all button
                file_box.foreach((child) => {
                    file_box.select_child(child as Gtk.FlowBoxChild);
                });
            });

            this.unselect_all_button.clicked.connect(() => { // Unselect all button
                file_box.foreach((child) => {
                    file_box.unselect_child(child as Gtk.FlowBoxChild);
                });
            });
        }
    } // End class
} // End namespace
