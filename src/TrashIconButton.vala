namespace TrashApplet { 
    
    public class TrashIconButton : Gtk.Button {

        private Gtk.Image? icon_empty = null;
        private Gtk.Image? icon_full = null;

        public TrashIconButton() {
            this.icon_empty = new Gtk.Image.from_icon_name("user-trash-symbolic", Gtk.IconSize.MENU);
            this.icon_full = new Gtk.Image.from_icon_name("user-trash-full-symbolic", Gtk.IconSize.MENU);

            this.set_image(icon_empty);

            this.tooltip_text = "Trash";

            this.get_style_context().add_class("flat");
            this.get_style_context().remove_class("button");
        }

        public void update_icon(bool is_empty) {
            if (is_empty) {
                set_image(icon_empty);
            } else {
                set_image(icon_full);
            }
        }
    } // End class
} // End namespace
