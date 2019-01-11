namespace TrashApplet { 

    public class TrashItem : Gtk.Box {

        /* State */
        private TrashHandler trash_handler;
        private bool restoring = false;

        public string file_path { get; private set; }
        public string file_name { get; private set; }

        /* Widgets */
        private Gtk.Image? file_icon = null;
        private Gtk.Label? display_text = null;
        private Gtk.Button? restore_button = null;
        private Gtk.Button? delete_button = null;
        private Gtk.Button? go_back_button = null;
        private Gtk.Button? confirm_button = null;

        /* Signals */
        public signal void on_delete(string file_name);
        public signal void on_restore(string file_name, string restore_path);

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
            display_text = new Gtk.Label(file_name);
            display_text.max_width_chars = 30;
            display_text.ellipsize = Pango.EllipsizeMode.END;
            display_text.halign = Gtk.Align.START;
            display_text.justify = Gtk.Justification.LEFT;

            restore_button = new Gtk.Button.from_icon_name("edit-undo-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            restore_button.tooltip_text = "Restore item";
            delete_button = new Gtk.Button.from_icon_name("user-trash-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            delete_button.tooltip_text = "Delete item";

            go_back_button = new Gtk.Button.with_label("No");
            confirm_button = new Gtk.Button.with_label("Yes");

            this.tooltip_text = file_path;

            apply_button_styles();
            connect_signals();

            pack_start(file_icon, false, false, 5);
            pack_start(display_text, true, true, 0);
            pack_end(confirm_button, false, false, 0);
            pack_end(go_back_button, false, false, 0);
            pack_end(delete_button, false, false, 0);
            pack_end(restore_button, false, false, 0);
            show_all();

            go_back_button.hide();
            confirm_button.hide();
        }

        private void apply_button_styles() {
            restore_button.get_style_context().add_class("flat");
            delete_button.get_style_context().add_class("flat");
            go_back_button.get_style_context().add_class("flat");
            confirm_button.get_style_context().add_class("flat");

            restore_button.get_style_context().remove_class("button");
            delete_button.get_style_context().remove_class("button");
            go_back_button.get_style_context().remove_class("button");
            confirm_button.get_style_context().remove_class("button");
        }

        private void connect_signals() {
            delete_button.clicked.connect(() => { // Delete button clicked
                show_confirmation(false);
            });

            restore_button.clicked.connect(() => { // Restore button clicked
                show_confirmation(true);
            });

            confirm_button.clicked.connect(() => { // Confirm button clicked
                if (restoring) { // User clicked the restore button
                    on_restore(file_name, file_path);
                } else { // User clicked the delete button
                    on_delete(file_name);
                }
            });

            go_back_button.clicked.connect(() => { // Go back button clicked
                show_item_display();
            });
        }

        private void show_confirmation(bool restore) {
            this.restoring = restore;

            if (restore) {
                display_text.label = "Really restore this file?";
            } else {
                display_text.label = "Really delete this file?";
            }

            restore_button.hide();
            delete_button.hide();

            go_back_button.show();
            confirm_button.show();
        }

        private void show_item_display() {
            display_text.label = file_name;

            go_back_button.hide();
            confirm_button.hide();

            restore_button.show();
            delete_button.show();
        }
    } // End class
} // End namespace
