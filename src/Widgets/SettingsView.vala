namespace TrashApplet.Widgets {

    public enum SortType {
        ALPHABETICAL,
        NEWEST_FIRST,
        OLDEST_FIRST,
        REVERSE_ALPHABETICAL,
        TYPE
    }

    public class SettingsView : Gtk.Box {

        private MainPopover popover;

        public SortType sort_type { get; private set; }

        private Gtk.Box? title_header = null;
        private Gtk.Label? title_label = null;
        private Gtk.ScrolledWindow? scroller = null;

        private Gtk.Box? settings_box = null;
        private Gtk.Box? sorting_section = null;
        private Gtk.Label? sorting_header = null;
        private Gtk.RadioButton? sort_alphabetical_button = null;
        private Gtk.RadioButton? sort_reverse_alphabetical_button = null;
        private Gtk.RadioButton? sort_oldest_button = null;
        private Gtk.RadioButton? sort_newest_button = null;
        private Gtk.RadioButton? sort_type_button = null;

        private Gtk.Box? footer = null;
        private Gtk.Button? return_button = null;

        public SettingsView(MainPopover popover) {
            Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);
            this.popover = popover;

            // Header
            title_header = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            title_header.height_request = 32;
            title_header.get_style_context().add_class("trash-applet-header");
            title_label = new Gtk.Label("Trash Settings");
            title_header.pack_start(title_label, true, true, 0);

            // Scroll area
            scroller = new Gtk.ScrolledWindow(null, null);
            scroller.min_content_height = 300;
            scroller.max_content_height = 300;
            scroller.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC);

            settings_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            settings_box.height_request = 300;
            settings_box.get_style_context().add_class("trash-settings-box");

            // Sorting section
            sorting_section = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            sorting_header = new Gtk.Label("Sort");
            sorting_header.halign = Gtk.Align.START;
            sorting_header.justify = Gtk.Justification.LEFT;

            sort_type_button = new Gtk.RadioButton.with_label(null, "Type");
            sort_alphabetical_button = new Gtk.RadioButton.with_label_from_widget(sort_type_button, "A-Z");
            sort_reverse_alphabetical_button = new Gtk.RadioButton.with_label_from_widget(sort_alphabetical_button, "Z-A");
            sort_oldest_button = new Gtk.RadioButton.with_label_from_widget(sort_alphabetical_button, "Oldest First");
            sort_newest_button = new Gtk.RadioButton.with_label_from_widget(sort_alphabetical_button, "Newest First");

            sort_type_button.set_active(true);
            sort_type = SortType.TYPE;

            sorting_section.pack_start(sorting_header, false, false, 0);
            sorting_section.pack_start(sort_type_button, false, false, 0);
            sorting_section.pack_start(sort_alphabetical_button, false, false, 0);
            sorting_section.pack_start(sort_reverse_alphabetical_button, false, false, 0);
            sorting_section.pack_start(sort_oldest_button, false, false, 0);
            sorting_section.pack_start(sort_newest_button, false, false, 0);

            settings_box.pack_start(sorting_section);

            scroller.add(settings_box);

            // Footer
            footer = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            footer.height_request = 32;
            footer.get_style_context().add_class("trash-applet-footer");
            return_button = new Gtk.Button.from_icon_name("edit-undo-symbolic", Gtk.IconSize.BUTTON);
            return_button.tooltip_text = "Return to main view";
            return_button.get_style_context().add_class("flat");
            return_button.get_style_context().remove_class("button");
            footer.pack_start(return_button, true, false, 0);

            // Put it all together
            pack_start(title_header, false, false, 0);
            pack_start(scroller);
            pack_end(footer);

            connect_signals();
        }

        private void connect_signals() {
            sort_type_button.clicked.connect(() => {
                foreach (var store in popover.get_trash_store_widgets()) {
                    sort_type = SortType.TYPE;
                    store.set_sort_type(sort_type);
                }
            });
            
            sort_alphabetical_button.clicked.connect(() => {
                foreach (var store in popover.get_trash_store_widgets()) {
                    sort_type = SortType.ALPHABETICAL;
                    store.set_sort_type(sort_type);
                }
            });

            sort_reverse_alphabetical_button.clicked.connect(() => {
                foreach (var store in popover.get_trash_store_widgets()) {
                    sort_type = SortType.REVERSE_ALPHABETICAL;
                    store.set_sort_type(sort_type);
                }
            });

            sort_newest_button.clicked.connect(() => {
                foreach (var store in popover.get_trash_store_widgets()) {
                    sort_type = SortType.NEWEST_FIRST;
                    store.set_sort_type(sort_type);
                }
            });

            sort_oldest_button.clicked.connect(() => {
                foreach (var store in popover.get_trash_store_widgets()) {
                    sort_type = SortType.OLDEST_FIRST;
                    store.set_sort_type(sort_type);
                }
            });

            return_button.clicked.connect(() => {
                popover.set_page("main");
            });
        }
    }
}
