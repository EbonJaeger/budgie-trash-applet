#include "trash_item.h"
#include "trash_item_ui.h"

/*
 * GObject functions
 */

G_DEFINE_TYPE(BtaTrashItem, bta_trash_item, GTK_TYPE_BOX);

static void bta_trash_item_init(BtaTrashItem *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void bta_trash_item_class_init(BtaTrashItemClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(widget_class, "/com/github/EbonJaeger/BudgieTrashApplet/BtaTrashItem.glade");
}

/*
 * End of GObject functions
 */

GtkWidget *bta_trash_item_new(void *ctx)
{
    return g_object_new(BTA_TYPE_TRASH_ITEM,
                        "context", ctx,
                        NULL);
}

void bta_toggle_revealer(GtkRevealer *revealer)
{
    gboolean revealed = gtk_revealer_get_child_revealed(revealer);
    gtk_revealer_set_reveal_child(revealer, !revealed);
}

void on_item_cancel(BtaTrashItem *self)
{
    bta_toggle_revealer(self->confirm_revealer);

    gtk_widget_set_sensitive((GtkWidget *)self->delete_button, TRUE);
    gtk_widget_set_sensitive((GtkWidget *)self->restore_button, TRUE);
}

void on_item_confirm(BtaTrashItem *self)
{
    // TODO: Delete or restore the item
}

void on_item_delete(BtaTrashItem *self)
{
    gtk_widget_set_sensitive((GtkWidget *)self->delete_button, FALSE);
    gtk_widget_set_sensitive((GtkWidget *)self->restore_button, FALSE);

    gtk_label_set_label(self->revealer_text, "Really delete this item?");
    bta_toggle_revealer(self->confirm_revealer);
}

void on_item_restore(BtaTrashItem *self)
{
    gtk_widget_set_sensitive((GtkWidget *)self->delete_button, FALSE);
    gtk_widget_set_sensitive((GtkWidget *)self->restore_button, FALSE);

    gtk_label_set_label(self->revealer_text, "Really restore this item?");
    bta_toggle_revealer(self->confirm_revealer);
}
