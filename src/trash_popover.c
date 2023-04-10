#include "trash_popover.h"

enum {
    TRASH_RESPONSE_EMPTY = 1,
    TRASH_RESPONSE_RESTORE
};

struct _TrashPopover {
    GtkBox parent_instance;

    TrashManager *trash_manager;

    GtkWidget *stack;
    GtkWidget *file_box;
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

static void trash_added(TrashManager *manager, TrashInfo *trash_info, TrashPopover *self) {
    (void) manager;

    TrashItemRow *row;

    row = trash_item_row_new(trash_info);

    gtk_list_box_insert(GTK_LIST_BOX(self->file_box), GTK_WIDGET(row), -1);
}

static void foreach_item_cb(TrashItemRow *row, gchar *uri) {
    TrashInfo *info;
    g_autofree const gchar *info_uri;

    info = trash_item_row_get_info(row);
    info_uri = trash_info_get_uri(info);

    if (g_strcmp0(info_uri, uri) == 0) {
        gtk_widget_destroy(GTK_WIDGET(row));
    }
}

static void trash_removed(TrashManager *manager, gchar *name, TrashPopover *self) {
    (void) manager;

    gtk_container_foreach(GTK_CONTAINER(self->file_box), (GtkCallback) foreach_item_cb, name);
}

static void selected_rows_changed(GtkListBox *source, gpointer user_data) {
    GtkInfoBar *info_bar = user_data;
    GList *selected_rows;
    guint count;

    selected_rows = gtk_list_box_get_selected_rows(source);
    count = g_list_length(selected_rows);

    gtk_info_bar_set_response_sensitive(info_bar, TRASH_RESPONSE_RESTORE, count > 0);
    g_list_free(selected_rows);
}

static void restore_item(gpointer data, gpointer user_data) {
    (void) user_data;
    TrashItemRow *row = data;

    trash_item_row_restore(row);
}

static void handle_response(GtkInfoBar *source, gint response, gpointer user_data) {
    (void) source;
    TrashPopover *self = user_data;
    GList *selected_rows;

    switch (response) {
        case TRASH_RESPONSE_RESTORE:
            selected_rows = gtk_list_box_get_selected_rows(GTK_LIST_BOX(self->file_box));
            g_list_foreach(selected_rows, restore_item, NULL);
            g_list_free(selected_rows);
            break;
        case TRASH_RESPONSE_EMPTY:
            break;
    }
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
    GtkWidget *info_bar, *action_area, *content_area, *btn;
    TrashSettings *settings_view;

    gtk_widget_set_size_request(GTK_WIDGET(self), -1, 256);

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

    // Create our main view

    info_bar = gtk_info_bar_new();
    action_area = gtk_info_bar_get_action_area(GTK_INFO_BAR(info_bar));
    gtk_orientable_set_orientation(GTK_ORIENTABLE(action_area), GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(action_area), GTK_BUTTONBOX_SPREAD);
    gtk_widget_set_hexpand(action_area, TRUE);

    content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR(info_bar));
    gtk_widget_destroy(content_area);

    btn = gtk_info_bar_add_button(GTK_INFO_BAR(info_bar), "Restore", TRASH_RESPONSE_RESTORE);
    gtk_widget_set_tooltip_text(btn, "Restore selected items");
    gtk_widget_set_hexpand(btn, TRUE);

    btn = gtk_info_bar_add_button(GTK_INFO_BAR(info_bar), "Empty", TRASH_RESPONSE_EMPTY);
    gtk_widget_set_tooltip_text(btn, "Empty the trash bin");
    gtk_widget_set_hexpand(btn, TRUE);

    g_signal_connect(info_bar, "response", G_CALLBACK(handle_response), self);

    // Create our drive list box
    self->file_box = gtk_list_box_new();
    gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(self->file_box), FALSE);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(self->file_box), GTK_SELECTION_MULTIPLE);

    g_signal_connect(self->file_box, "selected-rows-changed", G_CALLBACK(selected_rows_changed), info_bar);

    // Create our scrolled window
    scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroller), 256);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(scroller), self->file_box);

    main_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    selected_rows_changed(GTK_LIST_BOX(self->file_box), info_bar);
    gtk_container_add(GTK_CONTAINER(main_view), info_bar);
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

    g_signal_connect(self->trash_manager, "trash-added", G_CALLBACK(trash_added), self);
    g_signal_connect(self->trash_manager, "trash-removed", G_CALLBACK(trash_removed), self);

    trash_manager_scan_items(self->trash_manager);

    // Pack ourselves up
    gtk_box_pack_start(GTK_BOX(self), header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), separator, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(self), self->stack, TRUE, TRUE, 0);
    gtk_widget_show_all(GTK_WIDGET(self));
    gtk_stack_set_visible_child_name(GTK_STACK(self->stack), "main");
    gtk_widget_show_all(self->stack);
}

TrashPopover *trash_popover_new() {
    return g_object_new(TRASH_TYPE_POPOVER, "orientation", GTK_ORIENTATION_VERTICAL, "spacing", 0, NULL);
}
