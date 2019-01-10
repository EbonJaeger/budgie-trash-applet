namespace TrashApplet {

    public class TrashPopover : Budgie.Popover {

        private HashTable<string, TrashItem> trash_bin_items;
        private TrashHandler trash_handler;

        /* Widgets */
        private Gtk.Stack? stack = null;
        private Gtk.Box? main_view = null;
        private Gtk.Box? title_area = null;
        private Gtk.Label? title_label = null;
        private Gtk.Box? items_count_area = null;
        private Gtk.Label? items_count = null;
        private Gtk.ScrolledWindow? scroller = null;
        private Gtk.ListBox? file_box = null;
        private Gtk.Box? controls_area = null;

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

            this.title_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            this.title_area.height_request = 32;
            this.title_label = new Gtk.Label("Trash");
            this.title_area.pack_start(title_label, true, true, 0);

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

            scroller.add(file_box);

            this.controls_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);

            this.restore_button = new Gtk.Button.from_icon_name("edit-undo-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.restore_button.set_tooltip_text("Restore all items");

            this.delete_button = new Gtk.Button.from_icon_name("user-trash-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.delete_button.set_tooltip_text("Delete all items");

            this.controls_area.pack_start(restore_button);
            this.controls_area.pack_end(delete_button);

            this.main_view.pack_start(title_area);
            this.main_view.pack_start(new Gtk.Separator(Gtk.Orientation.HORIZONTAL));
            this.main_view.pack_start(items_count_area);
            this.main_view.pack_start(scroller);
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

        /**
         * Add a new trash item.
         * 
         * @param info The FileInfo for the newly trashed file
         * @param old_path The path that the file came from
         */
        public void add_trash_item(string file_name, string file_path, GLib.Icon file_icon) {
            var item = new TrashItem(trash_handler, file_path, file_name, file_icon);
            trash_bin_items.insert(file_name, item);
            file_box.insert(item, -1);
            set_count_label();
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
        }

        private void apply_button_styles() {
            restore_button.get_style_context().add_class("flat");
            delete_button.get_style_context().add_class("flat");

            restore_button.get_style_context().remove_class("button");
            delete_button.get_style_context().remove_class("button");
        }

        private void connect_signals() {
            /* Buttons */

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
    } // End class
} // End namespace
