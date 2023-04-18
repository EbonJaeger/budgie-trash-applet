#include "trash_icon_button.h"

struct _TrashIconButton {
    GtkButton parent_instance;

    GtkWidget *empty_image;
    GtkWidget *full_image;
};

G_DEFINE_TYPE(TrashIconButton, trash_icon_button, GTK_TYPE_BUTTON);

static void trash_icon_button_finalize(GObject *object) {
    TrashIconButton *self;

    self = TRASH_ICON_BUTTON(object);

    if (self->empty_image != NULL) {
        g_object_unref(self->empty_image);
    }

    if (self->full_image != NULL) {
        g_object_unref(self->full_image);
    }

    G_OBJECT_CLASS(trash_icon_button_parent_class)->finalize(object);
}

static void trash_icon_button_class_init(TrashIconButtonClass *klass) {
    GObjectClass *class;
    
    class = G_OBJECT_CLASS(klass);
    class->finalize = trash_icon_button_finalize;
}

static void trash_icon_button_init(TrashIconButton *self) {
    GtkStyleContext *style;
    
    style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style, "flat");
    gtk_style_context_remove_class(style, "button");

    self->empty_image = gtk_image_new_from_icon_name("user-trash-symbolic", GTK_ICON_SIZE_MENU);
    self->full_image = gtk_image_new_from_icon_name("user-trash-full-symbolic", GTK_ICON_SIZE_MENU);

    gtk_button_set_image(GTK_BUTTON(self), g_object_ref(self->empty_image));
    gtk_widget_set_tooltip_text(GTK_WIDGET(self), "Trash");

    gtk_widget_show_all(GTK_WIDGET(self));
}

/**
 * trash_icon_button_new:
 *
 * Creates a new #TrashIconButton object.
 *
 * Returns: a new #TrashIconButton object.
 */
TrashIconButton *trash_icon_button_new(void) {
    return g_object_new(TRASH_TYPE_ICON_BUTTON, NULL);
}

/**
 * trash_icon_button_set_filled:
 * @self: a #TrashIconButton
 *
 * Sets the filled trash bin image on the button.
 */
void trash_icon_button_set_filled(TrashIconButton *self) {
    GtkWidget *dup;
    
    dup = g_object_ref(self->full_image);
    gtk_button_set_image(GTK_BUTTON(self), dup);
}

/**
 * trash_icon_button_set_empty:
 * @self: a #TrashIconButton
 *
 * Sets the empty trash bin image on the button.
 */
void trash_icon_button_set_empty(TrashIconButton *self) {
    GtkWidget *dup;
    
    dup = g_object_ref(self->empty_image);
    gtk_button_set_image(GTK_BUTTON(self), dup);
}
