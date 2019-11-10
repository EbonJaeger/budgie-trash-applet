namespace TrashApplet.Widgets {

    public enum SortType {
        NAME,
        DATE
    }

    public class MainPopover : Budgie.Popover {

        private TrashHandler trash_handler;
        private HashTable<string, TrashStoreWidget> trash_stores;
        private SortType sort_type;

        /* Widgets */
        private Gtk.Stack? stack = null;
        private Gtk.Box? main_view = null;
        private Gtk.Box? title_header = null;
        private Gtk.Label? title_label = null;
        private Gtk.ScrolledWindow? scroller = null;
        private Gtk.ListBox? drive_box = null;

        private Gtk.Box? controls_box = null;
        private Gtk.RadioButton? sort_name_button = null;
        private Gtk.RadioButton? sort_date_button = null;

        /**
         * Constructor
         */
        public MainPopover(Gtk.Widget? parent, TrashHandler trash_handler) {
            Object(relative_to: parent);
            this.trash_handler = trash_handler;
            this.trash_stores = new HashTable<string, TrashStoreWidget>(str_hash, str_equal);
            width_request = 300;

            /* Views */
            this.stack = new Gtk.Stack();
            stack.set_transition_type(Gtk.StackTransitionType.SLIDE_LEFT_RIGHT);

            this.main_view = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

            title_header = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            title_header.height_request = 32;
            title_header.get_style_context().add_class("trash-applet-header");
            title_label = new Gtk.Label("Trash");
            title_header.pack_start(title_label, true, true, 0);

            scroller = new Gtk.ScrolledWindow(null, null);
            scroller.min_content_height = 300;
            scroller.max_content_height = 300;
            scroller.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC);

            drive_box = new Gtk.ListBox();
            drive_box.height_request = 300;
            drive_box.get_style_context().add_class("trash-applet-list");
            drive_box.activate_on_single_click = true;
            drive_box.selection_mode = Gtk.SelectionMode.NONE;

            scroller.add(drive_box);

            controls_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            controls_box.height_request = 32;
            controls_box.get_style_context().add_class("trash-applet-controls");
            sort_name_button = new Gtk.RadioButton.with_label(null, "A-Z");
            sort_name_button.tooltip_text = "Sort files alphabetically";
            sort_date_button = new Gtk.RadioButton.with_label_from_widget(sort_name_button, "Date");
            sort_date_button.tooltip_text = "Sort files by deletion time";

            sort_name_button.set_active(true);
            sort_type = SortType.NAME;
            controls_box.pack_start(sort_name_button, false, false, 0);
            controls_box.pack_start(sort_date_button, false, false, 0);

            main_view.pack_start(title_header, false, false, 0);
            main_view.pack_start(scroller);
            main_view.pack_end(controls_box);
            /* End view creation */

            stack.add_named(main_view, "main");

            this.stack.show_all();

            connect_signals();

            add(this.stack);

            // Look for any starting stores
            if (trash_handler.get_trash_stores().length() > 0) {
                trash_handler.get_trash_stores().foreach((trash_store) => {
                    var store_widget = new TrashStoreWidget(trash_store, sort_type);
                    drive_box.insert(store_widget, -1);
                    trash_stores.insert(trash_store.get_drive_name(), store_widget);
                });
            }
        }

        public void set_page(string page) {
            this.stack.set_visible_child_name(page);
        }

        private void connect_signals() {
            trash_handler.trash_store_added.connect((trash_store) => { // Trash store was added
                var store_widget = new TrashStoreWidget(trash_store, sort_type);
                trash_store.get_current_trash_items();
                drive_box.insert(store_widget, -1);
                trash_stores.insert(trash_store.get_drive_name(), store_widget);
            });

            trash_handler.trash_store_removed.connect((trash_store) => { // Trash store was removed
                drive_box.foreach((store) => {
                    if (store.get_name() == trash_store.get_drive_name()) {
                        drive_box.remove(store);
                        return;
                    }
                });
                var store = trash_stores.get(trash_store.get_drive_name());
                drive_box.remove(store.get_parent());
                trash_stores.remove(trash_store.get_drive_name());
            });

            sort_name_button.clicked.connect(() => {
                foreach (var store in trash_stores.get_values()) {
                    sort_type = SortType.NAME;
                    store.set_sort_type(sort_type);
                }
            });

            sort_date_button.clicked.connect(() => {
                foreach (var store in trash_stores.get_values()) {
                    sort_type = SortType.DATE;
                    store.set_sort_type(sort_type);
                }
            });
        }
    } // End class
} // End namespace
