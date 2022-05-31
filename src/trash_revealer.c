#include "trash_revealer.h"

static void trash_revealer_finalize (GObject *obj);

struct TrashRevealerPrivate {
    GtkWidget *container;

    GtkWidget *label;
    GtkWidget *cancel_button;
    GtkWidget *confirm_button;

    gboolean destructive;
    gchar *message;
};

typedef struct _ResponseData ResponseData;
struct _ResponseData {
    gint response_id;
};

enum {
    RESPONSE,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_DESTRUCTIVE,
    PROP_MESSAGE
};

static guint trash_revealer_signals [LAST_SIGNAL];

static void trash_revealer_response (TrashRevealer *self, gint response_id) {
    int new_response = response_id;

    g_return_if_fail (TRASH_IS_REVEALER (self));

    if (!(response_id == TRASH_REVEALER_RESPONSE_OK || response_id == TRASH_REVEALER_RESPONSE_CANCEL)) {
        return;
    }

    g_signal_emit (
        self,
        trash_revealer_signals [RESPONSE],
        0,
        new_response
    );
}

G_DEFINE_TYPE_WITH_PRIVATE (TrashRevealer, trash_revealer, GTK_TYPE_REVEALER);

static void trash_revealer_style_set (GtkWidget *widget, GtkStyle *previous_style) {
    TrashRevealer *self;

    if (GTK_WIDGET_CLASS (trash_revealer_parent_class)->style_set) {
        GTK_WIDGET_CLASS (trash_revealer_parent_class)->style_set (widget, previous_style);
    }

    self = TRASH_REVEALER (widget);

    gtk_revealer_set_transition_type (GTK_REVEALER (self), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_reveal_child (GTK_REVEALER (self), FALSE);

    gtk_widget_set_size_request (self->priv->label, 290, 20);
    gtk_label_set_line_wrap (GTK_LABEL (self->priv->label), TRUE);
}

static void trash_revealer_set_message (TrashRevealer *self, const gchar *message) {
    g_return_if_fail (TRASH_IS_REVEALER (self));

    g_free (self->priv->message);

    self->priv->message = g_strdup (message);

    if (self->priv->label) {
        if (self->priv->message) {
            gtk_label_set_markup (GTK_LABEL (self->priv->label), self->priv->message);
        }
    }
}

static void trash_revealer_set_destructive (TrashRevealer *self, gboolean destructive) {
    GtkStyleContext *confirm_style = gtk_widget_get_style_context(self->priv->confirm_button);

    if (destructive) {
        gtk_style_context_remove_class(confirm_style, "suggested-action");
        gtk_style_context_add_class(confirm_style, "destructive-action");
    } else {
        gtk_style_context_remove_class(confirm_style, "destructive-action");
        gtk_style_context_add_class(confirm_style, "suggested-action");
    }
}

static void
trash_revealer_get_property (
    GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec
) {
    TrashRevealer *self = TRASH_REVEALER (object);

    switch (prop_id)
    {
    case PROP_DESTRUCTIVE:
        g_value_set_boolean (value, self->priv->destructive);
        break;
    case PROP_MESSAGE:
        g_value_set_string (value, self->priv->message);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
trash_revealer_set_property (
    GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec
) {
    TrashRevealer *self = TRASH_REVEALER (object);

    switch (prop_id)
    {
    case PROP_DESTRUCTIVE:
        trash_revealer_set_destructive (self, g_value_get_boolean (value));
        break;
    case PROP_MESSAGE:
        trash_revealer_set_message (self, g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void trash_revealer_class_init (TrashRevealerClass *klass) {
    GObjectClass *class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    class->finalize = trash_revealer_finalize;
    class->get_property = trash_revealer_get_property;
    class->set_property = trash_revealer_set_property;
    
    widget_class->style_set = trash_revealer_style_set;

    trash_revealer_signals [RESPONSE] = g_signal_new (
        "response",
        G_OBJECT_CLASS_TYPE (klass),
        G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET (TrashRevealerClass, response),
        NULL, NULL,
        g_cclosure_marshal_VOID__INT,
        G_TYPE_NONE, 1,
        G_TYPE_INT
    );

    g_object_class_install_property (
        class,
        PROP_DESTRUCTIVE,
        g_param_spec_boolean (
            "destructive",
            "Destructive",
            "Reflects if the confirm button should have the destructive style",
            FALSE,
            G_PARAM_READWRITE
        )
    );

    g_object_class_install_property (
        class,
        PROP_MESSAGE,
        g_param_spec_string (
            "message",
            "Message",
            "The message shown by the revealer",
            NULL,
            G_PARAM_READWRITE
        )
    );
}

static void cancel_button_clicked (__attribute__((unused)) GtkButton *button, TrashRevealer *self) {
    trash_revealer_response (self, TRASH_REVEALER_RESPONSE_CANCEL);
}

static void confirm_button_clicked (__attribute__((unused)) GtkButton *button, TrashRevealer *self) {
    trash_revealer_response (self, TRASH_REVEALER_RESPONSE_OK);
}

static void trash_revealer_init (TrashRevealer *self) {
    self->priv = trash_revealer_get_instance_private (self);

    self->priv->container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    self->priv->label = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX (self->priv->container), self->priv->label, TRUE, TRUE, 0);

    GtkWidget *revealer_btns = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    self->priv->cancel_button = gtk_button_new_with_label ("No");
    self->priv->confirm_button = gtk_button_new_with_label ("Yes");

    GtkStyleContext *cancel_style = gtk_widget_get_style_context (self->priv->cancel_button);
    gtk_style_context_add_class (cancel_style, "flat");
    gtk_style_context_remove_class (cancel_style, "button");
    GtkStyleContext *confirm_style = gtk_widget_get_style_context (self->priv->confirm_button);
    gtk_style_context_add_class (confirm_style, "flat");
    gtk_style_context_remove_class (confirm_style, "button");

    gtk_box_pack_start (GTK_BOX (revealer_btns), self->priv->cancel_button, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (revealer_btns), self->priv->confirm_button, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (self->priv->container), revealer_btns, TRUE, TRUE, 0);

    // Signals
    g_signal_connect (
        self->priv->cancel_button,
        "clicked",
        G_CALLBACK (cancel_button_clicked),
        self
    );

    g_signal_connect (
        self->priv->confirm_button,
        "clicked",
        G_CALLBACK (confirm_button_clicked),
        self
    );

    if (self->priv->label) {
        if (self->priv->message) {
            gtk_label_set_markup (GTK_LABEL (self->priv->label), self->priv->message);
        }
    }

    // Pack ourselves up
    gtk_container_add (GTK_CONTAINER (self), self->priv->container);
    gtk_widget_show_all (GTK_WIDGET (self->priv->container));
}

static void trash_revealer_finalize (GObject *obj) {
    g_return_if_fail (obj != NULL);
    g_return_if_fail (TRASH_IS_REVEALER (obj));

    TrashRevealer *self = TRASH_REVEALER (obj);

    g_return_if_fail (self->priv != NULL);

    if (self->priv->message != NULL) {
        g_free (self->priv->message);
    }

    G_OBJECT_CLASS (trash_revealer_parent_class)->finalize (obj);
}

TrashRevealer *trash_revealer_new() {
    return g_object_new(TRASH_TYPE_REVEALER, NULL);
}

void trash_revealer_show_message (TrashRevealer *self, const gchar *message, gboolean destructive) {
    g_return_if_fail (TRASH_IS_REVEALER (self));

    trash_revealer_set_message (self, message ? message : "");
    trash_revealer_set_destructive (self, destructive);
}
