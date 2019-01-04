namespace TrashApplet.Widgets { 

private class IndicatorIcon : Gtk.Stack {

    private Gtk.Image? icon_empty = null;
    private Gtk.Image? icon_full = null;

    public IndicatorIcon() {
        icon_empty = new Gtk.Image.from_icon_name("user-trash-symbolic", Gtk.IconSize.MENU);
        icon_full = new Gtk.Image.from_icon_name("user-trash-full-symbolic", Gtk.IconSize.MENU);

        this.add_named(icon_empty, "empty");
        this.add_named(icon_full, "full");

        show_all();

        this.set_visible_child_name("empty");
    }
} // End class

} // End namespace
