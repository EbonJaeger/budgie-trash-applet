#include "trash_revealer.h"

enum {
    SIGNAL_CANCEL_CLICKED,
    SIGNAL_CONFIRM_CLICKED,
    N_SIGNALS
};

static guint confirmation_revealer_signals[N_SIGNALS] = {0};

enum {
    PROP_EXP_0,
    PROP_TEXT,
    N_EXP_PROPERTIES
};

static GParamSpec *revealer_props[N_EXP_PROPERTIES] = {
    NULL,
};

struct _TrashRevealer {
    GtkRevealer parent_instance;
    GtkWidget *container;

    gchar *text;

    GtkWidget *label;
    GtkWidget *cancel_button;
    GtkWidget *confirm_button;
};

struct _TrashRevealerClass {
    GtkRevealerClass parent_class;

    void (*cancel_clicked)(TrashRevealer *revealer);
    void (*confirm_clicked)(TrashRevealer *revealer);
};

G_DEFINE_TYPE(TrashRevealer, trash_revealer, GTK_TYPE_REVEALER);

static void trash_revealer_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec);
static void trash_revealer_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec);

static void trash_revealer_class_init(TrashRevealerClass *klazz) {
    GObjectClass *class = G_OBJECT_CLASS(klazz);

    confirmation_revealer_signals[SIGNAL_CANCEL_CLICKED] = g_signal_new(
        "cancel-clicked",
        G_TYPE_FROM_CLASS(class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
        G_STRUCT_OFFSET(TrashRevealerClass, cancel_clicked),
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        0,
        NULL);

    confirmation_revealer_signals[SIGNAL_CONFIRM_CLICKED] = g_signal_new(
        "confirm-clicked",
        G_TYPE_FROM_CLASS(class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
        G_STRUCT_OFFSET(TrashRevealerClass, confirm_clicked),
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        0,
        NULL);

    class->get_property = trash_revealer_get_property;
    class->set_property = trash_revealer_set_property;

    revealer_props[PROP_TEXT] = g_param_spec_string(
        "text",
        "Text",
        "Text to use in the revealer body",
        "",
        G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

    g_object_class_install_properties(class, N_EXP_PROPERTIES, revealer_props);
}

static void trash_revealer_get_property(GObject *obj, guint prop_id, GValue *val, GParamSpec *spec) {
    TrashRevealer *self = TRASH_REVEALER(obj);

    switch (prop_id) {
        case PROP_TEXT:
            g_value_set_string(val, self->text);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_revealer_set_property(GObject *obj, guint prop_id, const GValue *val, GParamSpec *spec) {
    TrashRevealer *self = TRASH_REVEALER(obj);

    if (!GTK_IS_LABEL(self->label)) {
        self->label = gtk_label_new("");
        g_object_set(G_OBJECT(self->label), "height-request", 20, NULL);
        gtk_box_pack_start(GTK_BOX(self->container), self->label, TRUE, TRUE, 0);
    }

    switch (prop_id) {
        case PROP_TEXT:
            trash_revealer_set_text(self, g_strdup(g_value_get_string(val)));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, spec);
            break;
    }
}

static void trash_revealer_init(TrashRevealer *self) {
    gtk_revealer_set_transition_type(GTK_REVEALER(self), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self), FALSE);

    self->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    self->label = gtk_label_new("");
    g_object_set(G_OBJECT(self->label), "height-request", 20, NULL);
    gtk_box_pack_start(GTK_BOX(self->container), self->label, TRUE, TRUE, 0);

    GtkWidget *revealer_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    self->cancel_button = gtk_button_new_with_label("No");
    self->confirm_button = gtk_button_new_with_label("Yes");

    GtkStyleContext *cancel_style = gtk_widget_get_style_context(self->cancel_button);
    gtk_style_context_add_class(cancel_style, "flat");
    gtk_style_context_remove_class(cancel_style, "button");
    GtkStyleContext *confirm_style = gtk_widget_get_style_context(self->confirm_button);
    gtk_style_context_add_class(confirm_style, "flat");
    gtk_style_context_add_class(confirm_style, "destructive-action");
    gtk_style_context_remove_class(confirm_style, "button");

    g_signal_connect_object(GTK_BUTTON(self->cancel_button), "clicked", G_CALLBACK(trash_revealer_handle_clicked), self, 0);
    g_signal_connect_object(GTK_BUTTON(self->confirm_button), "clicked", G_CALLBACK(trash_revealer_handle_clicked), self, 0);

    gtk_box_pack_start(GTK_BOX(revealer_btns), self->cancel_button, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(revealer_btns), self->confirm_button, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(self->container), revealer_btns, TRUE, TRUE, 0);

    // Pack ourselves up
    gtk_container_add(GTK_CONTAINER(self), self->container);
}

TrashRevealer *trash_revealer_new() {
    return g_object_new(TRASH_TYPE_REVEALER, NULL);
}

void trash_revealer_handle_clicked(GtkButton *sender, TrashRevealer *self) {
    if (sender == GTK_BUTTON(self->cancel_button)) {
        g_signal_emit(self, confirmation_revealer_signals[SIGNAL_CANCEL_CLICKED], 0, NULL);
    } else {
        g_signal_emit(self, confirmation_revealer_signals[SIGNAL_CONFIRM_CLICKED], 0, NULL);
    }
}

void trash_revealer_set_text(TrashRevealer *self, gchar *text) {
    gchar *text_clone = g_strdup(text);

    if (text_clone == NULL || strcmp(text_clone, "") == 0) {
        return;
    }

    // Free existing text if it is different
    if ((self->text) && strcmp(self->text, text_clone) != 0) {
        g_free(self->text);
    }

    self->text = text_clone;

    // Set the label text
    gtk_label_set_markup(GTK_LABEL(self->label), self->text);

    g_object_notify_by_pspec(G_OBJECT(self), revealer_props[PROP_TEXT]);
}
