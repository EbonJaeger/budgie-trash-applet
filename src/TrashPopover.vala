namespace TrashApplet {

    public class TrashPopover : Budgie.Popover {

        /* Widgets */
        private Gtk.Stack? stack = null;
        private Gtk.Box? main_view = null;
        private Gtk.Box? title_area = null;
        private Gtk.Label? title_label = null;
        private Gtk.Box? items_count_area = null;
        private Gtk.Label? items_count = null;
        private Gtk.FlowBox? file_box = null;
        private Gtk.Box? controls_area = null;

        private Gtk.Separator? horizontal_separator = null;

        private Gtk.Button? select_all_button = null;
        private Gtk.Button? restore_button = null;
        private Gtk.Button? delete_button = null;

        /**
         * Constructor
         */
        public TrashPopover(Gtk.Widget? parent) {
            Object(relative_to: parent);
            width_request = 600;

            /* Views */
            this.stack = new Gtk.Stack();

            this.main_view = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);

            this.title_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
            this.title_area.height_request = 32;
            this.title_label = new Gtk.Label("Trash");
            this.title_area.pack_start(title_label, true, true, 0);

            this.items_count_area = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
            this.items_count_area.height_request = 32;
            this.items_count = new Gtk.Label("Your trash bin is currently empty!");
            this.items_count_area.pack_start(items_count, true, true, 0);

            this.file_box = new Gtk.FlowBox();
            this.file_box.height_request = 200;
            this.file_box.activate_on_single_click = true;
            this.file_box.homogeneous = true;
            this.file_box.max_children_per_line = 6;

            this.controls_area = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);

            this.select_all_button = new Gtk.Button.from_icon_name("edit-select-all-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.select_all_button.set_tooltip_text("Select All");

            this.restore_button = new Gtk.Button.from_icon_name("edit-undo-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.restore_button.set_tooltip_text("Restore Items");

            this.delete_button = new Gtk.Button.from_icon_name("user-trash-symbolic", Gtk.IconSize.SMALL_TOOLBAR);
            this.delete_button.set_tooltip_text("Delete Items");

            this.controls_area.pack_start(select_all_button);
            this.controls_area.pack_start(restore_button);
            this.controls_area.pack_end(delete_button);

            this.horizontal_separator = new Gtk.Separator(Gtk.Orientation.HORIZONTAL);

            this.main_view.pack_start(title_area);
            this.main_view.pack_start(horizontal_separator);
            this.main_view.pack_start(items_count_area);
            this.main_view.pack_start(file_box);
            this.main_view.pack_start(horizontal_separator);
            this.main_view.pack_end(controls_area);

            apply_button_styles();

            this.stack.add_named(main_view, "main");

            this.stack.show_all();
            add(this.stack);
        }

        public void set_page(string page) {
            this.stack.set_visible_child_name(page);
        }

        public void apply_button_styles() {
            select_all_button.get_style_context().add_class("flat");
            restore_button.get_style_context().add_class("flat");
            delete_button.get_style_context().add_class("flat");

            select_all_button.get_style_context().remove_class("button");
            restore_button.get_style_context().remove_class("button");
            delete_button.get_style_context().remove_class("button");
        }
    } // End class
} // End namespace
