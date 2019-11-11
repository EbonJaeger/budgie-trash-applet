using Gtk;

namespace TrashApplet.Widgets {

    public class MainPopover : Budgie.Popover {

        private TrashHandler trash_handler;
        private HashTable<string, TrashStoreWidget> trash_stores;

        /* Widgets */
        private Stack? stack = null;
        private Box? main_view = null;
        private Box? title_header = null;
        private Label? title_label = null;
        private ScrolledWindow? scroller = null;
        private ListBox? drive_box = null;

        private Box? footer = null;
        private Button? settings_button = null;

        private SettingsView? settings_view = null;

        /**
         * Constructor
         */
        public MainPopover(Widget? parent, TrashHandler trash_handler) {
            Object(relative_to: parent);
            this.trash_handler = trash_handler;
            this.trash_stores = new HashTable<string, TrashStoreWidget>(str_hash, str_equal);
            width_request = 300;

            /* Views */
            this.stack = new Stack();
            stack.set_transition_type(StackTransitionType.SLIDE_LEFT_RIGHT);

            this.main_view = new Box(Orientation.VERTICAL, 0);

            title_header = new Box(Orientation.HORIZONTAL, 0);
            title_header.height_request = 32;
            title_header.get_style_context().add_class("trash-applet-header");
            title_label = new Label("Trash");
            title_header.pack_start(title_label, true, true, 0);

            scroller = new ScrolledWindow(null, null);
            scroller.min_content_height = 300;
            scroller.max_content_height = 300;
            scroller.set_policy(PolicyType.NEVER, PolicyType.AUTOMATIC);

            drive_box = new ListBox();
            drive_box.height_request = 300;
            drive_box.get_style_context().add_class("trash-applet-list");
            drive_box.activate_on_single_click = true;
            drive_box.selection_mode = SelectionMode.NONE;

            scroller.add(drive_box);

            footer = new Box(Orientation.HORIZONTAL, 0);
            footer.height_request = 32;
            footer.get_style_context().add_class("trash-applet-footer");
            settings_button = new Button.from_icon_name("emblem-system-symbolic", IconSize.BUTTON);
            settings_button.tooltip_text = "Go to applet options";
            settings_button.get_style_context().add_class("flat");
            settings_button.get_style_context().remove_class("button");
            footer.pack_start(settings_button, true, false, 0);

            main_view.pack_start(title_header, false, false, 0);
            main_view.pack_start(scroller);
            main_view.pack_end(footer);
            /* End view creation */

            settings_view = new SettingsView(this);

            stack.add_named(main_view, "main");
            stack.add_named(settings_view, "settings");

            stack.show_all();

            connect_signals();

            add(stack);

            // Look for any starting stores
            if (trash_handler.get_trash_stores().length() > 0) {
                trash_handler.get_trash_stores().foreach((trash_store) => {
                    var store_widget = new TrashStoreWidget(trash_store, settings_view.sort_type);
                    drive_box.insert(store_widget, -1);
                    trash_stores.insert(trash_store.get_drive_name(), store_widget);
                });
            }
        }

        public List<weak TrashStoreWidget> get_trash_store_widgets() {
            return trash_stores.get_values();
        }

        public void set_page(string page) {
            this.stack.set_visible_child_name(page);
        }

        private void connect_signals() {
            trash_handler.trash_store_added.connect((trash_store) => { // Trash store was added
                var store_widget = new TrashStoreWidget(trash_store, settings_view.sort_type);
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

            settings_button.clicked.connect(() => {
                set_page("settings");
            });
        }
    } // End class
} // End namespace
