namespace TrashApplet { 

    public class TrashItem : Gtk.Box {

        /* State */
        private string file_path;
        private string file_name;

        /* Widgets */
        private Gtk.Image? file_icon = null;
        private Gtk.Label? name_label = null;
        private Gtk.Button? restore_button = null;
        private Gtk.Button? delete_button = null;

        /**
         * Constructor
         */
        public TrashItem(string file_path, string file_name, GLib.Icon glib_icon) {
            Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);
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
    } // End class
} // End namespace
