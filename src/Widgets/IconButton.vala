namespace TrashApplet.Widgets {
    
    public class IconButton : Gtk.Button {

        private Gtk.Image? icon_empty = null;
        private Gtk.Image? icon_full = null;

        private TrashHandler trash_handler;

        public IconButton(TrashHandler trash_handler) {
            this.icon_empty = new Gtk.Image.from_icon_name("user-trash-symbolic", Gtk.IconSize.MENU);
            this.icon_full = new Gtk.Image.from_icon_name("user-trash-full-symbolic", Gtk.IconSize.MENU);
            this.trash_handler = trash_handler;

            this.set_image(icon_empty);

            this.tooltip_text = "Trash";

            this.get_style_context().add_class("flat");
            this.get_style_context().remove_class("button");

            // Hook up signals
            trash_handler.trash_added.connect(() => { // Trash was added
                set_image(icon_full);
            });

            trash_handler.trash_removed.connect((file_name, is_empty) => { // Trash was removed
                if (is_empty) {
                    set_image(icon_empty);
                }
            });
        }
    } // End class
} // End namespace
