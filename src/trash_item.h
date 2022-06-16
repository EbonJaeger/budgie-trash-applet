#pragma once

#include "notify.h"
#include "trash_confirm_dialog.h"
#include "trash_info.h"
#include "utils.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_ITEM (trash_item_get_type())

G_DECLARE_FINAL_TYPE(TrashItem, trash_item, TRASH, ITEM, GtkBox)

TrashItem *trash_item_new(TrashInfo *trash_info);

/**
 * Compares the name of the TrashItem with the given string.
 *
 * Returns `TRUE` if the name is the same.
 */
gint trash_item_has_name(TrashItem *self, gchar *name);

void trash_item_toggle_info_revealer(TrashItem *self);

void trash_item_delete(TrashItem *self, GError *err);
void trash_item_restore(TrashItem *self, GError *err);

/**
 * Compares two TrashItems for sorting, putting them in order by deletion date
 * in ascending order.
 */
gint trash_item_collate_by_date(TrashItem *self, TrashItem *other);

/**
 * Compares two TrashItems for sorting, putting them in alphabetical order.
 */
gint trash_item_collate_by_name(TrashItem *self, TrashItem *other);

/**
 * Compares two TrashItems for sorting. This function uses the following rules:
 *
 * 1. Directories should be above regular files
 * 2. Directories should be sorted alphabetically
 * 3. Files should be sorted alphabetically
 */
gint trash_item_collate_by_type(TrashItem *self, TrashItem *other);

G_END_DECLS
