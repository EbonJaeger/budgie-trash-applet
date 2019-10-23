namespace TrashApplet.Widgets {

    public class MainPopover : Budgie.Popover {

        private TrashHandler trash_handler;
        private HashTable<string, TrashStoreWidget> trash_stores;

        /* Widgets */
        private Gtk.Stack? stack = null;
        private Gtk.Box? main_view = null;
        private Gtk.Box? title_header = null;
        private Gtk.Label? title_label = null;
        private Gtk.ScrolledWindow? scroller = null;
        private Gtk.ListBox? drive_box = null;

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

            this.main_view.pack_start(title_header, false, false, 0);
            this.main_view.pack_start(scroller);
            /* End view creation */

            stack.add_named(main_view, "main");

            this.stack.show_all();

            connect_signals();

            add(this.stack);

            // Look for any starting stores
            if (trash_handler.get_trash_stores().length() > 0) {
                trash_handler.get_trash_stores().foreach((trash_store) => {
                    var store_widget = new TrashStoreWidget(trash_store);
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
                var store_widget = new TrashStoreWidget(trash_store);
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
        }
    } // End class
} // End namespace
