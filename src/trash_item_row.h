#pragma once

#include "notify.h"
#include "trash_button_bar.h"
#include "trash_info.h"
#include <gtk/gtk.h>
#include <pango/pango.h>

G_BEGIN_DECLS

#define TRASH_TYPE_ITEM_ROW (trash_item_row_get_type())

G_DECLARE_FINAL_TYPE(TrashItemRow, trash_item_row, TRASH, ITEM_ROW, GtkListBoxRow)

TrashItemRow *trash_item_row_new(TrashInfo *trash_info);

TrashInfo *trash_item_row_get_info(TrashItemRow *self);

void trash_item_row_delete(TrashItemRow *self);

void trash_item_row_restore(TrashItemRow *self);

gint trash_item_row_collate_by_date(TrashItemRow *self, TrashItemRow *other);

gint trash_item_row_collate_by_name(TrashItemRow *self, TrashItemRow *other);

gint trash_item_row_collate_by_type(TrashItemRow *self, TrashItemRow *other);

G_END_DECLS
