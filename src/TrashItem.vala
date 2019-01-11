namespace TrashApplet { 

    public class TrashItem : Gtk.Box {

        /* State */
        private TrashHandler trash_handler;

        public string file_path { get; private set; }
        public string file_name { get; private set; }

        /* Widgets */
        private Gtk.Image? file_icon = null;
        private Gtk.Label? name_label = null;
        private Gtk.Button? restore_button = null;
        private Gtk.Button? delete_button = null;

        /**
         * Constructor
         */
        public TrashItem(TrashHandler trash_handler, string file_path, string file_name, GLib.Icon glib_icon) {
            Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);
            this.trash_handler = trash_handler;
            this.file_path = file_path;
            this.file_name = file_name;

            this.height_request = 32;
            this.margin = 0;

            /* Create Widget stuff */
            file_icon = new Gtk.Image.from_gicon(glib_icon, Gtk.IconSize.SMALL_TOOLBAR);
            name_label = new Gtk.Label(file_name);
            name_label.max_width_chars = 30;
            name_label.ellipsize = Pango.EllipsizeMode.END;
            name_label.halign = Gtk.Align.START;
            name_label.justify = Gtk.Justification.LEFT;

            restore_button = new Gtk.Button.from_icon_name("edit-undo-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            restore_button.tooltip_text = "Restore item";
            delete_button = new Gtk.Button.from_icon_name("user-trash-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            delete_button.tooltip_text = "Delete item";

            this.tooltip_text = file_path;

            apply_button_styles();
            connect_signals();

            pack_start(file_icon, false, false, 5);
            pack_start(name_label, true, true, 0);
            pack_end(delete_button, false, false, 0);
            pack_end(restore_button, false, false, 0);
            show_all();
        }

        private void apply_button_styles() {
            restore_button.get_style_context().add_class("flat");
            delete_button.get_style_context().add_class("flat");

            restore_button.get_style_context().remove_class("button");
            delete_button.get_style_context().remove_class("button");
        }

        private void connect_signals() {
            delete_button.clicked.connect(() => { // Delete button clicked
                trash_handler.delete_file(file_name);
            });
        }
    } // End class
} // End namespace
