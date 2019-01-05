namespace TrashApplet { 

    public class TrashItem : Gtk.Box {

        /* State */
        private string file_path;
        private string file_name;
        private FileType file_type;

        /* Widgets */
        private Gtk.Image? file_icon = null;
        private Gtk.Label? name_label = null;

        /**
         * Constructor
         */
        public TrashItem(FileInfo file, string old_path) {
            Object(orientation: Gtk.Orientation.VERTICAL, spacing: 0);
            
            this.file_path = old_path;
            this.file_name = file.get_name();
            this.file_type = file.get_file_type();

            /* Create Widget stuff */
            this.file_icon = new Gtk.Image.from_gicon(file.get_icon(), Gtk.IconSize.DIALOG);
            this.name_label = new Gtk.Label(file_name);
            this.name_label.max_width_chars = 16;
            this.name_label.ellipsize = Pango.EllipsizeMode.END;

            this.tooltip_text = file_path;

            pack_start(file_icon, true, true, 0);
            pack_end(name_label);
            show_all();
        }
    } // End class
} // End namespace
