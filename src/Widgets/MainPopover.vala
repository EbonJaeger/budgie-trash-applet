namespace TrashApplet.Widgets {

    public class MainPopover : Budgie.Popover {

        private TrashHandler trash_handler;

        /* Widgets */
        private Gtk.Stack? stack = null;
        private Gtk.Box? main_view = null;
        private Gtk.Label? title_label = null;
        private Gtk.ScrolledWindow? scroller = null;

        /**
         * Constructor
         */
        public MainPopover(Gtk.Widget? parent, TrashHandler trash_handler) {
            Object(relative_to: parent);
            this.trash_handler = trash_handler;
            width_request = 300;

            /* Views */
            this.stack = new Gtk.Stack();
            stack.set_transition_type(Gtk.StackTransitionType.SLIDE_LEFT_RIGHT);

            this.main_view = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

            scroller = new Gtk.ScrolledWindow(null, null);
            scroller.min_content_height = 300;
            scroller.max_content_height = 300;
            scroller.hscrollbar_policy = Gtk.PolicyType.NEVER;

            this.main_view.pack_start(generate_title_widget(), false, false, 0);
            this.main_view.pack_start(new Gtk.Separator(Gtk.Orientation.HORIZONTAL));
            this.main_view.pack_start(scroller);
            /* End view creation */

            stack.add_named(main_view, "main");

            this.stack.show_all();

            connect_signals();

            add(this.stack);

            // Look for any starting stores
            warning("Number of trash stores: %u", trash_handler.get_trash_stores().length());
            if (trash_handler.get_trash_stores().length() > 0) {
                trash_handler.get_trash_stores().foreach((trash_store) => {
                    var store_widget = new TrashStoreWidget(trash_store);
                    scroller.add(store_widget);
                });
            }
        }

        private Gtk.Box generate_title_widget() {
            var title_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            title_area.height_request = 32;
            title_label = new Gtk.Label("");
            title_label.set_markup("<b>Trash</b>");
            title_area.pack_start(title_label, true, true, 0);

            return title_area;
        }

        public void set_page(string page) {
            this.stack.set_visible_child_name(page);
        }

        private void connect_signals() {
            trash_handler.trash_store_added.connect((trash_store) => { // Trash store was added
                var store_widget = new TrashStoreWidget(trash_store);
                scroller.add(store_widget);
            });

            trash_handler.trash_store_removed.connect((trash_store) => { // Trash store was removed
                scroller.foreach((store) => {
                    if (store.get_name() == trash_store.get_drive_name()) {
                        scroller.remove(store);
                        return;
                    }
                });
            });
        }
    } // End class
} // End namespace
