#include "trash_confirm_dialog.h"

struct _TrashConfirmDialog {
    GtkRevealer parent_instance;

    gboolean destructive;
    gchar *message;
    
    GtkWidget *label;
    GtkWidget *cancel_button;
    GtkWidget *confirm_button;
};

enum {
    RESPONSE_SIGNAL,
    N_SIGNALS
};

enum {
    PROP_0,
    PROP_DESTRUCTIVE,
    PROP_MESSAGE,
    N_PROPS
};

static guint signals[N_SIGNALS];
static GParamSpec *props[N_PROPS];

G_DEFINE_TYPE(TrashConfirmDialog, trash_confirm_dialog, GTK_TYPE_REVEALER);

static void trash_confirm_dialog_finalize(GObject *object) {
    TrashConfirmDialog *self;

    self = TRASH_CONFIRM_DIALOG(object);

    if (self->message != NULL) {
        g_free(self->message);
    }

    G_OBJECT_CLASS(trash_confirm_dialog_parent_class)->finalize(object);
}

static void
trash_confirm_dialog_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    TrashConfirmDialog *self;

    self = TRASH_CONFIRM_DIALOG(object);

    switch (prop_id) {
        case PROP_DESTRUCTIVE:
            g_value_set_boolean(value, trash_confirm_dialog_get_destructive(self));
            break;
        case PROP_MESSAGE:
            g_value_set_string(value, trash_confirm_dialog_get_message(self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void trash_confirm_dialog_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    TrashConfirmDialog *self;

    self = TRASH_CONFIRM_DIALOG(object);

    switch (prop_id) {
        case PROP_DESTRUCTIVE:
            trash_confirm_dialog_set_destructive(self, g_value_get_boolean(value));
            break;
        case PROP_MESSAGE:
            trash_confirm_dialog_set_message(self, g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void trash_confirm_dialog_class_init(TrashConfirmDialogClass *klass) {
    GObjectClass *class;

    class = G_OBJECT_CLASS(klass);
    class->finalize = trash_confirm_dialog_finalize;
    class->get_property = trash_confirm_dialog_get_property;
    class->set_property = trash_confirm_dialog_set_property;

    // Signals

    signals[RESPONSE_SIGNAL] = g_signal_new(
        "response",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL, NULL, NULL,
        G_TYPE_NONE,
        1,
        G_TYPE_INT);

    // Properties

    props[PROP_DESTRUCTIVE] = g_param_spec_boolean(
        "destructive",
        "Destructive",
        "Reflects if the confirm button should have the destructive style",
        FALSE,
        G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    props[PROP_MESSAGE] = g_param_spec_string(
        "message",
        "Message",
        "The message shown by the revealer",
        NULL,
        G_PARAM_CONSTRUCT | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(class, N_PROPS, props);
}

static void cancel_button_clicked(GtkButton *button, TrashConfirmDialog *self) {
    (void) button;

    g_signal_emit(self, signals[RESPONSE_SIGNAL], 0, GTK_RESPONSE_CANCEL);
}

static void confirm_button_clicked(GtkButton *button, TrashConfirmDialog *self) {
    (void) button;

    g_signal_emit(self, signals[RESPONSE_SIGNAL], 0, GTK_RESPONSE_OK);
}

static void trash_confirm_dialog_init(TrashConfirmDialog *self) {
    GtkWidget *container;
    GtkWidget *button_box;
    GtkStyleContext *cancel_button_style;
    GtkStyleContext  *confirm_button_style;

    gtk_widget_set_margin_top(GTK_WIDGET(self), 12);

    gtk_revealer_set_transition_type(GTK_REVEALER(self), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child(GTK_REVEALER(self), FALSE);

    container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    self->label = gtk_label_new(NULL);
    gtk_label_set_line_wrap(GTK_LABEL(self->label), TRUE);
    gtk_box_pack_start(GTK_BOX(container), self->label, TRUE, TRUE, 0);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    self->cancel_button = gtk_button_new_with_label("No");
    self->confirm_button = gtk_button_new_with_label("Yes");

    cancel_button_style = gtk_widget_get_style_context(self->cancel_button);
    gtk_style_context_add_class(cancel_button_style, GTK_STYLE_CLASS_FLAT);

    confirm_button_style = gtk_widget_get_style_context(self->confirm_button);
    gtk_style_context_add_class(confirm_button_style, GTK_STYLE_CLASS_FLAT);

    gtk_box_pack_start(GTK_BOX(button_box), self->cancel_button, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(button_box), self->confirm_button, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(container), button_box, TRUE, TRUE, 0);

    // Signals
    g_signal_connect(
        self->cancel_button,
        "clicked",
        G_CALLBACK(cancel_button_clicked),
        self);

    g_signal_connect(
        self->confirm_button,
        "clicked",
        G_CALLBACK(confirm_button_clicked),
        self);

    // Pack ourselves up
    gtk_container_add(GTK_CONTAINER(self), container);
    gtk_widget_show_all(container);
}

/**
 * Create a new TrashConfirmDialog widget.
 */
TrashConfirmDialog *trash_confirm_dialog_new(const gchar *message, gboolean destructive) {
    return g_object_new(TRASH_TYPE_CONFIRM_DIALOG, "message", message, "destructive", destructive, NULL);
}

gboolean trash_confirm_dialog_get_destructive(TrashConfirmDialog *self) {
    return self->destructive;
}

const gchar *trash_confirm_dialog_get_message(TrashConfirmDialog *self) {
    return g_strdup(self->message);
}

void trash_confirm_dialog_set_destructive(TrashConfirmDialog *self, gboolean destructive) {
    GtkStyleContext *button_style;

    self->destructive = destructive;

    if (!self->confirm_button) {
        return;
    }

    button_style = gtk_widget_get_style_context(self->confirm_button);

    if (self->destructive) {
        gtk_style_context_remove_class(button_style, GTK_STYLE_CLASS_SUGGESTED_ACTION);
        gtk_style_context_add_class(button_style, GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
    } else {
        gtk_style_context_remove_class(button_style, GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
        gtk_style_context_add_class(button_style, GTK_STYLE_CLASS_SUGGESTED_ACTION);
    }
}

void trash_confirm_dialog_set_message(TrashConfirmDialog *self, const gchar *message) {
    if (self->message) {
        g_free(self->message);
    }

    self->message = g_strdup(message);

    if (!self->label) {
        return;
    }

    gtk_label_set_markup(GTK_LABEL(self->label), self->message);
}
