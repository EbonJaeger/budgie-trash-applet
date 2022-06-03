#pragma once

#include "applet.h"
#include <budgie-desktop/plugin.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRASH_TYPE_PLUGIN (trash_plugin_get_type())

G_DECLARE_FINAL_TYPE(TrashPlugin, trash_plugin, TRASH, PLUGIN, GObject)

struct _TrashPlugin {
    GObject parent;
};

GType trash_plugin_get_type(void);

G_END_DECLS
