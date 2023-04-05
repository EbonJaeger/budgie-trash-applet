#pragma once

#include "trash_settings.h"
#include <budgie-desktop/popover.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_POPOVER (trash_popover_get_type())

G_DECLARE_FINAL_TYPE(TrashPopover, trash_popover, TRASH, POPOVER, GtkBox)

TrashPopover *trash_popover_new();

G_END_DECLS
