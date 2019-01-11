namespace TrashApplet {

public class Plugin : GLib.Object, Budgie.Plugin {
    
    public Budgie.Applet get_panel_widget(string uuid) {
        return new Applet(uuid);
    }
}

public class Applet : Budgie.Applet {

    private Gtk.EventBox? event_box = null;
    private TrashIconButton? icon_button = null;
    private TrashPopover? popover = null;

    private TrashHandler? trash_handler = null;

    private unowned Budgie.PopoverManager? manager = null;

    public string uuid { public set; public get; }

    public Applet(string uuid) {
        GLib.Object(uuid: uuid);

        // Set up our trash handler
        this.trash_handler = new TrashHandler();

        // Create the main layout
        event_box = new Gtk.EventBox();
        this.icon_button = new TrashIconButton(trash_handler);
        event_box.add(icon_button);

        this.add(event_box);

        this.popover = new TrashPopover(icon_button, trash_handler);
        popover.set_page("main");

        trash_handler.get_current_trash_items();

        this.show_all();
        connect_signals();
    }

    public override bool supports_settings() {
        return false;
    }

    public override void update_popovers(Budgie.PopoverManager? manager) {
        manager.register_popover(icon_button, popover);
        this.manager = manager;
    }

    private void connect_signals() {
        this.icon_button.clicked.connect(() => { // Trash button was clicked
            if (popover.is_visible()) { // Hide popover if currently being shown
                popover.hide();
            } else {
                manager.show_popover(icon_button);
            }
        });
    }
}

} // End namespace

[ModuleInit]
public void peas_register_types(GLib.TypeModule module)
{
    Peas.ObjectModule objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(Budgie.Plugin), typeof(TrashApplet.Plugin));
}
