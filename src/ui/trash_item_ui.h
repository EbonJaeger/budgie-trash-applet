#ifndef _BTA_TRASH_ITEM_UI_H
#define _BTA_TRASH_ITEM_UI_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BTA_TYPE_TRASH_ITEM (bta_trash_item_get_type())
#define BTA_TRASH_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), BTA_TYPE_TRASH_ITEM, BtaTrashItem))
#define BTA_IS_TRASH_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), BTA_TYPE_TRASH_ITEM))

typedef struct _BtaTrashItem BtaTrashItem;
typedef struct _BtaTrashItemClass BtaTrashItemClass;

struct _BtaTrashItem
{
    GtkBox parent;

    GtkButton *delete_button;
    GtkButton *restore_button;

    GtkRevealer *info_revealer;

    GtkRevealer *confirm_revealer;
    GtkLabel *revealer_text;
    GtkButton *go_back_button;
    GtkButton *confirm_button;
};

struct _BtaTrashItemClass
{
    GtkBoxClass parent_class;
};

GType bta_trash_item_get_type(void);

GtkWidget *bta_trash_item_new(void *ctx);

void bta_toggle_revealer(GtkRevealer *revealer);

void on_item_cancel(BtaTrashItem *self);

void on_item_confirm(BtaTrashItem *self);

void on_item_delete(BtaTrashItem *self);

void on_item_restore(BtaTrashItem *self);

G_END_DECLS

#endif
