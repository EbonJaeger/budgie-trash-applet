namespace TrashApplet {

    public class TrashPopover : Budgie.Popover {

        private HashTable<string, TrashItem> trash_bin_items;
        private TrashHandler trash_handler;

        /* Widgets */
        private Gtk.Stack? stack = null;
        private Gtk.Box? main_view = null;
        private Gtk.Label? title_label = null;
        private Gtk.Box? items_count_area = null;
        private Gtk.Label? items_count = null;
        private Gtk.ScrolledWindow? scroller = null;
        private Gtk.ListBox? file_box = null;
        private Gtk.Box? controls_area = null;

        private Gtk.Box? confirmation_view = null;
        private Gtk.Label? confirmation_text = null;
        private Gtk.Box? confirmation_controls = null;
        private Gtk.Button? go_back_button = null;
        private Gtk.Button? confirm_delete_button = null;

        private Gtk.Button? restore_button = null;
        private Gtk.Button? delete_button = null;

        /**
         * Constructor
         */
        public TrashPopover(Gtk.Widget? parent, TrashHandler trash_handler) {
            Object(relative_to: parent);
            this.trash_handler = trash_handler;
            width_request = 300;

            this.trash_bin_items = new HashTable<string, TrashItem>(str_hash, str_equal);

            /* Views */
            this.stack = new Gtk.Stack();

            this.main_view = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

            this.items_count_area = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            this.items_count_area.height_request = 32;
            this.items_count = new Gtk.Label("Your trash bin is currently empty!");
            this.items_count_area.pack_start(items_count, true, true, 0);

            scroller = new Gtk.ScrolledWindow(null, null);
            scroller.min_content_height = 300;
            scroller.max_content_height = 300;
            scroller.hscrollbar_policy = Gtk.PolicyType.NEVER;

            this.file_box = new Gtk.ListBox();
            this.file_box.height_request = 300;
            this.file_box.activate_on_single_click = true;
            this.file_box.selection_mode = Gtk.SelectionMode.NONE;
            file_box.set_sort_func(sort_rows);

            scroller.add(file_box);

            this.controls_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);

            this.restore_button = new Gtk.Button.from_icon_name("edit-undo-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.restore_button.set_tooltip_text("Restore all items");

            this.delete_button = new Gtk.Button.from_icon_name("user-trash-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.delete_button.set_tooltip_text("Delete all items");

            this.controls_area.pack_start(restore_button);
            this.controls_area.pack_end(delete_button);

            this.main_view.pack_start(generate_title_widget(), false, false, 0);
            this.main_view.pack_start(new Gtk.Separator(Gtk.Orientation.HORIZONTAL));
            this.main_view.pack_start(items_count_area);
            this.main_view.pack_start(scroller);
            this.main_view.pack_end(controls_area);

            /* Delete confirmation view */
            confirmation_view = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

            confirmation_text = new Gtk.Label("Are you sure you want to delete all items from the trash?");
            confirmation_text.halign = Gtk.Align.CENTER;
            confirmation_text.justify = Gtk.Justification.LEFT;
            confirmation_text.wrap = true;
            confirmation_text.margin_start = 20;
            confirmation_text.margin_end = 20;

            confirmation_controls = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            confirmation_controls.height_request = 40;

            go_back_button = new Gtk.Button.with_label("No");
            confirm_delete_button = new Gtk.Button.with_label("Yes");

            confirmation_view.pack_start(generate_title_widget(), false, false, 0);
            confirmation_view.pack_start(new Gtk.Separator(Gtk.Orientation.HORIZONTAL), false, false, 0);
            confirmation_view.pack_start(confirmation_text, false, false, 20);
            confirmation_controls.pack_start(go_back_button);
            confirmation_controls.pack_end(confirm_delete_button);
            confirmation_view.pack_end(confirmation_controls, false, false, 0);

            /* End view creation */

            stack.add_named(main_view, "main");
            stack.add_named(confirmation_view, "confirmation");

            this.stack.show_all();

            apply_button_styles();
            set_buttons_sensitive(false);
            connect_signals();

            add(this.stack);
        }

        private Gtk.Box generate_title_widget() {
            var title_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            title_area.height_request = 32;
            title_label = new Gtk.Label("Trash");
            title_area.pack_start(title_label, true, true, 0);

            return title_area;
        }

        public void set_page(string page) {
            this.stack.set_visible_child_name(page);
        }

        /**
         * Add a new trash item.
         * 
         * @param info The FileInfo for the newly trashed file
         * @param old_path The path that the file came from
         */
        public void add_trash_item(string file_name, string file_path, GLib.Icon file_icon, bool is_directory) {
            var item = new TrashItem(trash_handler, file_path, file_name, file_icon, is_directory);
            trash_bin_items.insert(file_name, item);
            file_box.insert(item, -1);
            set_count_label();
            set_buttons_sensitive(true);

            item.on_delete.connect((file_name) => {
                trash_handler.delete_file(file_name);
            });

            item.on_restore.connect((file_name, restore_path) => {
                trash_handler.restore_file(file_name, restore_path);
            });
        }

        /**
         * Remove an item from the trash view.
         * 
         * @param file_name The name of the file to remove
         */
        public void remove_trash_item(string file_name, bool is_empty) {
            var item = trash_bin_items.get(file_name);
            this.file_box.remove(item.get_parent());
            this.trash_bin_items.remove(file_name);
            set_count_label();
            if (trash_bin_items.size() == 0) { // No items in trash; buttons should no longer be sensitive
                set_buttons_sensitive(false);
            }
        }

        private void apply_button_styles() {
            restore_button.get_style_context().add_class("flat");
            delete_button.get_style_context().add_class("flat");
            go_back_button.get_style_context().add_class("flat");
            confirm_delete_button.get_style_context().add_class("flat");

            restore_button.get_style_context().remove_class("button");
            delete_button.get_style_context().remove_class("button");
            go_back_button.get_style_context().remove_class("button");
            confirm_delete_button.get_style_context().remove_class("button");
        }

        private void set_buttons_sensitive(bool is_sensitive) {
            restore_button.sensitive = is_sensitive;
            delete_button.sensitive = is_sensitive;
        }

        private void connect_signals() {
            /* Buttons */
            delete_button.clicked.connect(() => { // Delete all button clicked
                set_page("confirmation");
            });

            go_back_button.clicked.connect(() => { // Go back button in confirmation dialog
                set_page("main");
            });

            confirm_delete_button.clicked.connect(() => { // Confirm deletion button in confirmation dialog
                trash_bin_items.get_values().foreach((item) => {
                    trash_handler.delete_file(item.file_name);
                });
                set_page("main");
            });

            restore_button.clicked.connect(() => { // Restore all button was clicked
                trash_bin_items.get_values().foreach((item) => {
                    trash_handler.restore_file(item.file_name, item.file_path);
                });
            });

            /* Trash signals */
            trash_handler.trash_added.connect(add_trash_item);
            trash_handler.trash_removed.connect(remove_trash_item);
        }

        private void set_count_label() {
            if (trash_bin_items.size() == 0) {
                this.items_count.label = "Your trash bin is currently empty!";
            } else {
                this.items_count.label = "Currently %u item(s) in trash.".printf(trash_bin_items.size());
            }
        }

        /**
         * sort_rows will determine the order that two given rows should be in.
         * 
         * Directories should be above files, and both types should be sorted alphabetically.
         * 
         * @param row1 The first row to use for comparison
         * @param row2 The second row to use for comparison
         * @return < 0 if row1 should be before row2, 0 if they are equal and > 0 otherwise
         */
        private int sort_rows(Gtk.ListBoxRow row1, Gtk.ListBoxRow row2) {
            var trash_item_1 = row1.get_child() as TrashItem;
            var trash_item_2 = row2.get_child() as TrashItem;

            if (trash_item_1.is_directory && trash_item_2.is_directory) { // Both items are directories, sort by name
                return trash_item_1.file_name.collate(trash_item_2.file_name);
            } else if (trash_item_1.is_directory && !trash_item_2.is_directory) { // First item is a directory
                return -1; // First item should be above the second
            } else if (!trash_item_1.is_directory && trash_item_2.is_directory) { // First item is a file, second is a directory
                return 1; // Second item should be above the first
            } else { // Both items are files, sort by file name
                return trash_item_1.file_name.collate(trash_item_2.file_name);
            }
        }
    } // End class
} // End namespace
