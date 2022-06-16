#pragma once

#include "trash_enum_types.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum {
    TRASH_SORT_TYPE = 1,
    TRASH_SORT_A_Z = 2,
    TRASH_SORT_Z_A = 3,
    TRASH_SORT_DATE_ASCENDING = 4,
    TRASH_SORT_DATE_DESCENDING = 5
} TrashSortMode;

/**
 * Constant ID for our settings gschema
 */
#define TRASH_SETTINGS_SCHEMA_ID "com.github.ebonjaeger.budgie-trash-applet"

#define TRASH_SETTINGS_KEY_SORT_MODE "sort-mode"

#define TRASH_TYPE_SETTINGS (trash_settings_get_type())

G_DECLARE_FINAL_TYPE(TrashSettings, trash_settings, TRASH, SETTINGS, GtkBox)

/**
 * Create a new TrashSettings class.
 */
TrashSettings *trash_settings_new();

/**
 * Get the current sort mode for trash items.
 */
TrashSortMode trash_settings_get_sort_mode(TrashSettings *self);

G_END_DECLS
