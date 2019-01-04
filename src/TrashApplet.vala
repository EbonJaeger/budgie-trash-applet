namespace TrashApplet {

public class Plugin : GLib.Object, Budgie.Plugin {
    
    public Budgie.Applet get_panel_widget(string uuid) {
        return new Applet(uuid);
    }
}

public class Applet : Budgie.Applet {

    private Gtk.EventBox? event_box = null;

    public string uuid { public set; public get; }

    public Applet(string uuid) {
        GLib.Object(uuid: uuid);

        // Create the main layout
        event_box = new Gtk.EventBox();
        Widgets.IndicatorIcon icon = new Widgets.IndicatorIcon();
        event_box.add(icon);

        this.add(event_box);

        this.show_all();
    }

    public override bool supports_settings() {
        return false;
    }
}

} // End namespace

[ModuleInit]
public void peas_register_types(GLib.TypeModule module)
{
    Peas.ObjectModule objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(Budgie.Plugin), typeof(TrashApplet.Plugin));
}
