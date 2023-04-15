#include "trash_button_bar.h"

enum {
    RESPONSE,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

struct _TrashButtonBar {
    GtkBox parent_instance;
    
    GtkWidget *action_area;
};

typedef struct {
    gint response_id;
} ResponseData;

G_DEFINE_FINAL_TYPE(TrashButtonBar, trash_button_bar, GTK_TYPE_BOX)

static void trash_button_bar_class_init(TrashButtonBarClass *klass) {
    (void) klass;
    
    // Signals
    
    signals[RESPONSE] = g_signal_new("response",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_RUN_LAST,
                 0,
                 NULL, NULL, NULL,
                 G_TYPE_NONE,
                 1,
                 G_TYPE_INT);
}

static void trash_button_bar_init(TrashButtonBar *self) {
    GtkStyleContext *button_bar_style, *action_area_style;
    
    self->action_area = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    
    action_area_style = gtk_widget_get_style_context(self->action_area);
    gtk_style_context_add_class(action_area_style, "trash-button-bar-actions");
    
    gtk_box_pack_start(GTK_BOX(self), self->action_area, TRUE, TRUE, 6);
    
    button_bar_style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(button_bar_style, "trash-button-bar");
    
    gtk_widget_show_all(GTK_WIDGET(self));
}

TrashButtonBar *trash_button_bar_new(void) {
    return g_object_new(TRASH_TYPE_BUTTON_BAR, "orientation", GTK_ORIENTATION_VERTICAL, "spacing", 0, NULL);
}

static void response_data_free(gpointer data) {
    g_slice_free(ResponseData, data);
}

static ResponseData *get_response_data(GtkWidget *widget, gboolean create) {
    ResponseData *data;
    
    data = g_object_get_data(G_OBJECT(widget), "trash-button-bar-response-data");
    
    if (data == NULL && create) {
        data = g_slice_new(ResponseData);
        
        g_object_set_data_full(G_OBJECT(widget), "trash-button-bar-response-data", data, response_data_free);
    }
    
    return data;
}

static GtkWidget *find_button(TrashButtonBar *self, gint response_id) {
    GtkWidget *widget = NULL;
    GList *children, *list;
    
    children = gtk_container_get_children(GTK_CONTAINER(self->action_area));
    
    for (list = children; list; list = list->next) {
        ResponseData *data;
        
        data = get_response_data(list->data, FALSE);
        
        if (data && data->response_id == response_id) {
            widget = list->data;
            break;
        }
    }
    
    g_list_free(children);
    
    return widget;
}

static void button_clicked(GtkButton *button, gpointer user_data) {
    TrashButtonBar *self = user_data;
    ResponseData *data;

    data = get_response_data(GTK_WIDGET(button), FALSE);
    
    g_signal_emit(self, signals[RESPONSE], 0, data->response_id);
}

/**
 * Adds a new button to the bar with the given label and response id.
 *
 * The resulting widget is returned, though you generally don't need it.
 */
GtkWidget *trash_button_bar_add_button(TrashButtonBar *self, const gchar *text, gint response_id) {
    GtkWidget *button;
    ResponseData *data;
    
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(text != NULL, NULL);
    
    button = gtk_button_new_with_label(text);
    gtk_button_set_use_underline(GTK_BUTTON(button), TRUE);
    
    // Set the response data to the button
    data = get_response_data(button, TRUE);
    data->response_id = response_id;
    
    g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), self);
    
    gtk_box_pack_start(GTK_BOX(self->action_area), button, TRUE, TRUE, 6);
    
    gtk_widget_show(button);
    
    return button;
}

/**
 * Sets the sensitivity of the button that has the given `response_id`.
 */
void trash_button_bar_set_response_sensitive(TrashButtonBar *self, gint response_id, gboolean sensitive) {
    GtkWidget *widget;
    
    g_return_if_fail(self != NULL);
    
    widget = find_button(self, response_id);
    
    if (widget == NULL) {
        g_warning("Could not find widget for response id");
        return;
    }
    
    gtk_widget_set_sensitive(widget, sensitive);
}
