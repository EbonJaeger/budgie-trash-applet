#pragma once

#include "trash_store.h"
#include <gio/gio.h>
#include <unistd.h>

G_BEGIN_DECLS

#define TRASH_TYPE_MANAGER (trash_manager_get_type())

G_DECLARE_FINAL_TYPE(TrashManager, trash_manager, TRASH, MANAGER, GObject)

TrashManager *trash_manager_new(void);

G_END_DECLS
