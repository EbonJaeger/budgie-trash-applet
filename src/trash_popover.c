#include "trash_popover.h"

struct _TrashPopover {
    GtkBox parent_instance;

    TrashManager *trash_manager;

    GtkWidget *stack;
    GtkWidget *drive_list;
};

G_DEFINE_TYPE(TrashPopover, trash_popover, GTK_TYPE_BOX)

static void trash_popover_finalize(GObject *object) {
    TrashPopover *self;

    self = TRASH_POPOVER(object);

    g_object_unref(self->trash_manager);

    G_OBJECT_CLASS(trash_popover_parent_class)->finalize(object);
}

static void trash_popover_class_init(TrashPopoverClass *klass) {
    GObjectClass *class;

    class = G_OBJECT_CLASS(klass);
    class->finalize = trash_popover_finalize;
}

static void settings_clicked(GtkButton *button, TrashPopover *self) {
    GtkStack *stack;
    GtkWidget *image;
    const gchar *current_name = NULL;

    stack = GTK_STACK(self->stack);
    current_name = gtk_stack_get_visible_child_name(stack);

    if (g_strcmp0(current_name, "main") == 0) {
        gtk_stack_set_visible_child_name(stack, "settings");

        image = gtk_image_new_from_icon_name("drive-multidisk-symbolic", GTK_ICON_SIZE_BUTTON);
        gtk_button_set_image(button, image);
        gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Drives");
    } else {
        gtk_stack_set_visible_child_name(stack, "main");

        image = gtk_image_new_from_icon_name("system-settings-symbolic", GTK_ICON_SIZE_BUTTON);
        gtk_button_set_image(button, image);
        gtk_widget_set_tooltip_text(GTK_WIDGET(button), "Settings");
    }
}

static void bin_added(TrashManager *manager, TrashStore *bin, __attribute__((unused)) TrashPopover *self) {
    (void) manager;

    g_message("trash bin added: %s", trash_store_get_name(bin));
}

static void bin_removed(TrashManager *manager, gchar *bin_name, __attribute__((unused)) TrashPopover *self) {
    (void) manager;

    g_message("trash bin removed: %s", bin_name);
}

static void trash_popover_init(TrashPopover *self) {
    GtkWidget *header;
    GtkWidget *header_label;
    GtkWidget *settings_button;
    GtkStyleContext *header_label_style;
    GtkStyleContext *settings_button_style;
    GtkWidget *separator;
    GtkWidget *main_view;
    GtkWidget *scroller;
    TrashSettings *settings_view;

    // Create our header
    header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(header, 4);
    gtk_widget_set_margin_end(header, 4);

    // TODO: Text attribute to make the label bold

    header_label = gtk_label_new("Trash");
    gtk_widget_set_halign(header_label, GTK_ALIGN_START);
    gtk_widget_set_margin_start(header_label, 4);

    header_label_style = gtk_widget_get_style_context(header_label);
    gtk_style_context_add_class(header_label_style, GTK_STYLE_CLASS_DIM_LABEL);

    settings_button = gtk_button_new_from_icon_name("preferences-system-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(settings_button, "Trash Applet Settings");
    g_signal_connect(settings_button, "clicked", G_CALLBACK(settings_clicked), self);

    settings_button_style = gtk_widget_get_style_context(settings_button);
    gtk_style_context_add_class(settings_button_style, GTK_STYLE_CLASS_FLAT);
    gtk_style_context_remove_class(settings_button_style, GTK_STYLE_CLASS_BUTTON);

    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    // Pack up the header
    gtk_box_pack_start(GTK_BOX(header), header_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(header), settings_button, FALSE, FALSE, 0);

    // Create our drive list box
    self->drive_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(self->drive_list), GTK_SELECTION_NONE);

    // Create our scrolled window
    scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroller), 300);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(scroller), self->drive_list);

    // Create our main view
    main_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_view), scroller);
    gtk_widget_show_all(main_view);

    // Create our settings view
    settings_view = trash_settings_new();

    // Create our stack
    self->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(self->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_add_named(GTK_STACK(self->stack), main_view, "main");
    gtk_stack_add_named(GTK_STACK(self->stack), GTK_WIDGET(settings_view), "settings");

    // Trash Manager hookups

    self->trash_manager = trash_manager_new();

    g_signal_connect(self->trash_manager, "trash-bin-added", G_CALLBACK(bin_added), self);
    g_signal_connect(self->trash_manager, "trash-bin-removed", G_CALLBACK(bin_removed), self);

    // Pack ourselves up
    gtk_box_pack_start(GTK_BOX(self), header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(self), separator, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(self), self->stack, TRUE, TRUE, 0);
    gtk_widget_show_all(GTK_WIDGET(self));
    gtk_stack_set_visible_child_name(GTK_STACK(self->stack), "main");
    gtk_widget_show_all(self->stack);
}

TrashPopover *trash_popover_new() {
    return g_object_new(TRASH_TYPE_POPOVER, "orientation", GTK_ORIENTATION_VERTICAL, "spacing", 0, NULL);
}
