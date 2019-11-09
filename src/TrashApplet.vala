using GLib;
using TrashApplet.Widgets;

namespace TrashApplet {

public class Plugin : Object, Budgie.Plugin {
    
    public Budgie.Applet get_panel_widget(string uuid) {
        return new Applet(uuid);
    }
}

public class Applet : Budgie.Applet {

    private const Gtk.TargetEntry[] targets = {
        { "text/uri-list", 0, 0 }
    };

    private Gtk.EventBox? event_box = null;
    private IconButton? icon_button = null;
    private MainPopover? popover = null;

    private TrashHandler? trash_handler = null;

    private unowned Budgie.PopoverManager? manager = null;

    public string uuid { public set; public get; }

    public Applet(string uuid) {
        Object(uuid: uuid);

        // Notify.init("budgie-trash-applet");

        // Set up our trash handler
        this.trash_handler = new TrashHandler(this);

        // Load CSS styling
        Gdk.Screen screen = this.get_display().get_default_screen();
        Gtk.CssProvider provider = new Gtk.CssProvider();
        string style_file = "/com/github/EbonJaeger/budgie-trash-applet/style/style.css";
        Timeout.add(1000, () => {
            provider.load_from_resource(style_file);
            Gtk.StyleContext.add_provider_for_screen(screen, provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
            return false;
        });

        // Create the main layout
        event_box = new Gtk.EventBox();
        this.icon_button = new IconButton(trash_handler);
        event_box.add(icon_button);

        this.add(event_box);

        this.popover = new MainPopover(icon_button, trash_handler);
        popover.set_page("main");

        trash_handler.get_current_trash_items();

        this.show_all();
        Gtk.drag_dest_set(event_box, Gtk.DestDefaults.ALL, targets, Gdk.DragAction.COPY);
        connect_signals();
    }

    public override bool supports_settings() {
        return false;
    }

    public override void update_popovers(Budgie.PopoverManager? manager) {
        manager.register_popover(icon_button, popover);
        this.manager = manager;
    }

    public void show_notification(string summary, string body) {
        // I hate this and want to do this more programmaticly with libnotify.
        var cmd = "notify-send -a 'Budgie Trash Applet' -i user-trash-symbolic '%s' '%s'".printf(summary, body);
        Process.spawn_command_line_async(cmd);

        /*
        var notification = new Notify.Notification(summary, body, "user-trash-symbolic");
        notification.set_app_name("Budgie Trash Applet");
        notification.set_urgency(Notify.Urgency.NORMAL);

        try {
            notification.show();
        } catch (Error e2) {
            warning("Unable to send notification: %s".printf(e2.message));
        }
        */
    }

    private void connect_signals() {
        this.icon_button.clicked.connect(() => { // Trash button was clicked
            if (popover.is_visible()) { // Hide popover if currently being shown
                popover.hide();
            } else {
                manager.show_popover(icon_button);
            }
        });

        event_box.drag_data_received.connect(on_drag_data_received);
    }

    private void on_drag_data_received(Gtk.Widget widget, Gdk.DragContext context, int x, int y, Gtk.SelectionData selection_data, uint item, uint time) {
        if (item != 0) { // We don't care about this target type
            return;
        }

        var data = (string) selection_data.get_data();
        if (data.has_prefix("file://")) {
            var path = data.replace("file://", "");
            path = path.replace("%20", " "); // Try to make a useable path
            path = path.strip();
            var file = File.new_for_path(path);

            try {
                file.trash();
            } catch (Error e) {
                warning("Unable to trash dragged file '%s': %s".printf(path, e.message));
                show_notification("Error moving to trash", e.message);
            }
        }

        Gtk.drag_finish(context, true, true, time);
    }
}

} // End namespace

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    Peas.ObjectModule objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(Budgie.Plugin), typeof(TrashApplet.Plugin));
}
